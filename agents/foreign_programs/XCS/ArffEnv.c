/*
/       (XCS-C 1.2)
/	------------------------------------
/	Learning Classifier System based on accuracy
/
/     by 
/     Martin V. Butz
/     Illinois Genetic Algorithms Laboratory (IlliGAL)
/     University of Illinois at Urbana/Champaign
/     butz@illigal.ge.uiuc.edu
/
/     Last modified: 09-30-2003
/
/     The environment for testing an .arff data file.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "ArffEnv.h"

struct dataAttribute **data;
int entNumTotal;
int entArraySize;
int attNum;
char *dataName;
char **attNames;
int currentChoice;
int arffIsTesting; /* determines if test set should be presented */
int *arffFoldEnds; /* saves the end position of each fold in the data */
int arffExpNumber;
int arffDo10Folded;
int arffTestProblemNr;

struct dataAttributeType *attTypes;
struct arffFuncProps funcProps;

/* "private" functions... */
int getCorrespondingIntegerValue(char *name, int whichAttribute);
int addDataEntry(char *s, int i);
struct dataAttributeType * getAttributeTypeItem(char *s, int *i);
struct dataAttributeType * generateIntegerAttribute(char *s, int i);
struct dataAttributeType * generateRealAttribute(char *s, int i);
struct dataAttributeType * generateNominalAttribute(char *s, int i);
int whichEntry(char *s, int *pos);
char *getName(char *s, int *pos);
void addAttribute(char *name, struct dataAttributeType *att);
void fprintDataAttribute(FILE *outfile, struct dataAttribute dA, int attribute);
void setArffState(struct attributes *state);

/*####################### construct and destruct the environment ############################*/


/**
 * Initialize the environment reading the provided data file that must be in .arff format.
 */ 
int initEnv(FILE *fp)
{
  char s[1000000];
  char *name;
  int i,data;
  struct dataAttributeType *dAT=0;

  printf("Reading Data File...\n");
  data=0;
  entNumTotal=0;
  entArraySize = 0;
  attNum=0;
  while(fscanf(fp,"%[^\n]\n",s)==1){
    /* printf("Reading: %s", s);*/

    for(i=0; s[i]!='\0'; i++)
      if(s[i]!=' ' && s[i]!='\t')
	break;

    if(s[i] == '%' || s[i] == '\0') {
      ; /* commented or empty line */
    }else{
      if(data==0) { /* parsing header */
	switch(whichEntry(s, &i)) {
	case 0: /* @relation */
	  printf("Reading Header...\n");
	  dataName = getName(s, &i);
	  break;
	case 1: /* @attribute */
	  printf("Reading Attribute...\n");
	  name = getName(s, &i);
	  dAT = getAttributeTypeItem(s, &i);
	  if(name!=NULL && dAT != NULL) {
	    addAttribute(name, dAT);
	  }else{
	    printf("Did not succeed in adding attribute\n%s\n", s);
	    return 0;
	  }
	  break;
	case 2: /* @data */
	  data = 1;
	  printf("Reading Data...\n");
	  break;
	default: /* unknown */
	  printf("did not succeed in parsing file\n");
	  return 0;
	}
      }else{ /* parsing data entry */
	addDataEntry(s, i);
      }
    }
  }
  /* assure that the last entry is a nominal!!! */
  if(attTypes[attNum-1].type != 3) {
    return 0;
  }
  currentChoice = 0;
  arffIsTesting=0;

  funcProps.addNoiseToAction = ARFF_ADD_NOISE_TO_ACTION;
  funcProps.actionNoiseMu = ARFF_ACTION_NOISE_MU;
  funcProps.actionNoiseSigma = ARFF_ACTION_NOISE_SIGMA;
  funcProps.paymentRange = ARFF_PAYMENT_RANGE;

  arffDo10Folded = 0;
  arffExpNumber = 0;

  return 1;
}

/**
 * Free the allocated memory.
 */
void freeEnv()
{
  int i;

  for(i=0; i<entNumTotal; i++){
    free(data[i]);
  }
  free(data);
  for(i=0; i<attNum; i++){
    free(attNames[i]);
  }
  free(attNames);
  free(attTypes);
  free(dataName);
}

/**
 * Returns the number of actions. 
 */
int getNumberOfActions()
{
  return (attTypes[attNum-1].upperBound)+1;
}

/**
 * Returns the lenght of a problem instance.
 */
int getConditionLength()
{
  return attNum-1; /* the last one is the class ! */
}

/**
 * Returns the type (boolean, int, real, nominal) 
 */
int getAttributeType(int att)
{
  if(attTypes[att].type == 3) /* nominals are handled like integers for now */
    return 1;
  if(attTypes[att].type == 4) /* numericals are handled like reals for now */
    return 2;
  return attTypes[att].type;
}

/**
 * Returns the upper bound of an attribute.
 */
double getUpperBound(int att)
{
  if(attTypes[att].type == 2 || attTypes[att].type == 4) /* real or numerical */
    return attTypes[att].upperRealBound;
  return attTypes[att].upperBound;
}

/**
 * Returns the lower bound of an attribute.
 */
double getLowerBound(int att)
{
  if(attTypes[att].type == 2 || attTypes[att].type == 4) /* real or numerical */
    return attTypes[att].lowerRealBound;
  return attTypes[att].lowerBound;
}

/**
 * Returns the payment range of this problem (i.e. the maximum payoff).
 */
int getPaymentRange()
{
  return funcProps.paymentRange;
}

/**
 * Resets the state to a new problem instance.
 * state has memory allocated to support one complete problem instance.
 */ 
void resetState(struct attributes *state)
{
  int entNumCurrent;

  if(! arffDo10Folded) {
    currentChoice = urand() * entNumTotal;
  }else{
    if(arffIsTesting) {
      entNumCurrent = arffFoldEnds[arffExpNumber];
      if(arffExpNumber>0) {
	entNumCurrent -= arffFoldEnds[arffExpNumber-1];
      }
      currentChoice = urand() * entNumCurrent;
      if(arffExpNumber>0) {
	currentChoice += arffFoldEnds[arffExpNumber-1];
      }
    }else{
      entNumCurrent = entNumTotal - arffFoldEnds[arffExpNumber];
      if(arffExpNumber>0) {
	entNumCurrent += arffFoldEnds[arffExpNumber-1];
      }
      currentChoice = urand() * entNumCurrent;
      if( arffExpNumber==0 || currentChoice >= arffFoldEnds[arffExpNumber-1]) {
	currentChoice += arffFoldEnds[arffExpNumber];
	if(arffExpNumber>0) {
	  currentChoice -= arffFoldEnds[arffExpNumber-1];
	}
      }
    }
  }  
  setArffState(state);
}

/** 
 * Execute one function classification (feedback in 'correct' and the reward value)
 */ 
double doAction(struct attributes *ss, int act, int *correct)
{
  double value=0;
  
  if(data[currentChoice][attNum-1].integer == act) {
    *correct = 1;
    value = funcProps.paymentRange;
  }else{
    *correct = 0;
    value = 0;
  }

  if(funcProps.addNoiseToAction>0) {
    if(funcProps.addNoiseToAction<4) { /* Add Gaussian noise to outcome value */
      if(  funcProps.addNoiseToAction==1 || (funcProps.addNoiseToAction==2 && act==0) || (funcProps.addNoiseToAction==3 && act==1))
	value += (funcProps.actionNoiseMu) + nrand() * (funcProps.actionNoiseSigma);
    }else{ /* Alternate outcome with a certain probabilitiy possibly action dependent */
      if(  funcProps.addNoiseToAction<=5 || (funcProps.addNoiseToAction<=6 && act==0) || (funcProps.addNoiseToAction>6 && funcProps.addNoiseToAction<=7 && act==1)) {
	if(urand() < funcProps.addNoiseToAction-(double)(int)(funcProps.addNoiseToAction)) {
	  if(value == funcProps.paymentRange)
	    value = 0;
	  else
	    value = funcProps.paymentRange;
	}
      }
    }
  }
  return value;
}

int getNextNiche(struct attributes *state, int type)
{
  /* nearly impossible to implement - if we would know, why bother with the data? */
  return 0;
}

/**
 * Returns the exact estimate for the given condition and action
 */
double getExactEstimate(struct condAttribute *conArray, int class, double *mad)
{
  int i,j;
  double av, all;
  struct attributes att;

  av=0, all=0;
  for(i=0; i<entNumTotal; i++) {
    if(arffDo10Folded && ( (arffExpNumber==0 && i==0) || 
			   (arffExpNumber>0 && i==arffFoldEnds[arffExpNumber-1]))
       ) {
      i = arffFoldEnds[arffExpNumber];
      if(i==entNumTotal)
	break;
    }

    for(j=0; j<attNum-1; j++) {
      att.type = data[i][j].type;
      att.bool = data[i][j].bool;
      att.integer = data[i][j].integer;
      att.real = data[i][j].real;
      if(att.type==3)
	att.type=1;
      else if(att.type==4)
	att.type=2;
      if(!doesMatchAttribute(conArray[j], att))
	break;
    }
    if(j==attNum-1) { /* matched */
      all++;
      if(class==data[i][j].integer) {
	av ++;
      }
    }
  }
  if(all==0) { /* classifier does not match any data */
    *mad = 1000;
    return 0;
  }
  av = av / all;
  *mad = 2000. * (av - av * av);
  return av*1000;
}


/**
 * Print the environment constants to file.
 */
void fprintEnv(FILE *outfile)
{
  int i,j;

  fprintf(outfile, "# Function %s; addNoiseToAction %f; actionNoiseMu %f; actionNoiseSigma %f\n", dataName, funcProps.addNoiseToAction, funcProps.actionNoiseMu, funcProps.actionNoiseSigma); 
  fprintf(outfile, "# paymentRange %f; conditionLength %d\n", funcProps.paymentRange, getConditionLength());
  
  i=0;
  j=0;

  /*
   * for(i=0; i<entNumTotal; i++) {
   * for(j=0; j<attNum-1; j++) {
   *   fprintDataAttribute(outfile, data[i][j], j);
   *   fprintf(outfile,",");
   * }
   * fprintDataAttribute(outfile, data[i][j], j);
   * fprintf(outfile,"\n");
   *}*/
  
}

void fprintDataAttribute(FILE *outfile, struct dataAttribute dA, int attribute)
{
  switch(dA.type) {
  case 0:
    fprintf(outfile, "%c" , dA.bool);
    break;
  case 1:
    fprintf(outfile, "%d", dA.integer);
    break;
  case 2:
    fprintf(outfile, "%f", dA.real);
    break;
  case 3:
    fprintf(outfile, "%s", (attTypes[attribute].nominals)[dA.integer]);
    break;
  default:
    printf("Could not read type %d\n",dA.type);
    break;
  }
} 

/**
 * Tries to set parameter 'type' to value 'value. If 'type' does not exist, 0 is returned.
 */
int setEnvParam(char *type, double value)
{
  if(strcmp(type,"addNoiseToAction")==0){
    funcProps.addNoiseToAction = value;
  }else if(strcmp(type,"actionNoiseMu")==0){
    funcProps.actionNoiseMu = value;
  }else if(strcmp(type,"actionNoiseSigma")==0){
    funcProps.actionNoiseSigma = value;
  }else if(strcmp(type,"paymentRange")==0){
    printf("Payment Range is set internally!\n");
    /*funcProps.paymentRange = (int)value;*/
  }else{
    printf("%s not supported in current environment\n",type);
    return 0;
  }
  return 1;
}


/* ##################################################################################
 * ################### input file parsing ###########################################
 * #################################################################################*/


/**
 * Returns the integer value for a nominal entry.
 */
int getCorrespondingIntegerValue(char *name, int whichAttribute)
{
  int i;

  for(i=0; i<attTypes[whichAttribute].upperBound+1; i++) {
    if(strcmp(name, (attTypes[whichAttribute].nominals)[i])==0)
      return i;
  }
  return -1;
}

/**
 * Parses another data entry (line) and adds it to the data array.
 */
int addDataEntry(char *s, int i)
{
  struct dataAttribute *dA;
  struct dataAttribute **help;
  int begin, end, att;
  char entry[100000];
  
  /* generate data entry */
  assert((dA = (struct dataAttribute *)calloc(attNum, sizeof(struct dataAttribute)))!=NULL); 

  /* read attributes */
  for(att=0; att<attNum; att++) {
    /* get one entry */
    begin=i;
    while(s[i]!=',' && s[i]!='\0')
      i++;
    end=i;
    strncpy(entry, &(s[begin]), end-begin);
    entry[end-begin]='\0';
    /* enter into array */
    if(strcmp(entry,"?")==0) {
      /* missing data base entry -> generating type 0 data attribute for now !!! */
      dA[att].type = 0;
      dA[att].bool = DONT_CARE;
    }else{
      switch(attTypes[att].type) {
      case 1: /* integral */
	dA[att].type = 1;
	dA[att].integer = getIntValue(entry);
	if(attTypes[att].upperBound < attTypes[att].lowerBound) { /* bounds were not set */
	  attTypes[att].lowerBound = dA[att].integer;
	  attTypes[att].upperBound = dA[att].integer;
	}else{
	  /* enlarging boundaries if necessary */
	  if(dA[att].integer < attTypes[att].lowerBound) 
	    attTypes[att].lowerBound = dA[att].integer;
	  if(dA[att].integer > attTypes[att].upperBound)
	    attTypes[att].upperBound = dA[att].integer;
	}
	break;
      case 2: /* real */
	dA[att].type = 2;
	dA[att].real = getDoubleValue(entry);
	if(attTypes[att].upperRealBound < attTypes[att].lowerRealBound) { /* bounds were not set */
	  attTypes[att].lowerRealBound = dA[att].real;
	  attTypes[att].upperRealBound = dA[att].real;
	}else{
	  /* enlarging boundaries if necessary */
	  if(dA[att].real < attTypes[att].lowerRealBound) 
	    attTypes[att].lowerRealBound = dA[att].real;
	  if(dA[att].real > attTypes[att].upperRealBound)
	    attTypes[att].upperRealBound = dA[att].real;
	}
	break;
      case 3: /* nominal */
	dA[att].type = 3;
	dA[att].integer = getCorrespondingIntegerValue(entry, att);
	if(dA[att].integer==-1) {
	  printf("Do not know this nominal entry!!! %s\n", entry);
	  return 0;
	}
	break;
      case 4: /* numerical */
	dA[att].type = 2;
	dA[att].real = getDoubleValue(entry);
	/* determine lower and upper bound online */
	if(entNumTotal==0) {
	  /* first entry in the data base */
	  attTypes[att].lowerRealBound = dA[att].real; 
	  attTypes[att].upperRealBound = dA[att].real; 
	}else{
	  if(dA[att].real < attTypes[att].lowerRealBound)
	    attTypes[att].lowerRealBound = dA[att].real;
	  if(dA[att].real > attTypes[att].upperRealBound)
	    attTypes[att].upperRealBound = dA[att].real;
	}
	break;
      default: 
	printf("Format not supported\n");
	free(dA);
	return 0;
	break;
      }
    }
    i++;
    while(s[i]==' ' || s[i]=='\t')
      i++;
  }
  if(att != attNum) {
    printf("Did not succeed in reading data entry:\n%s\n", s);
    free(dA);
    return 0;
  }

  /* add problem instance to database array */
  entNumTotal++;
  if(entNumTotal>entArraySize) {
    assert((help = (struct dataAttribute **)calloc(entArraySize+1000, sizeof(struct dataAttribute *)))!=NULL);
    for(i=0; i<entArraySize; i++)
      help[i] = data[i];
    free(data);
    data=help;
    entArraySize += 1000;
  }
  data[entNumTotal-1] = dA;
  return 1;
}

/**
 * Determines the type of this attribute from the header information 
 * after @attribute and the name of the attribute
 */
struct dataAttributeType *getAttributeTypeItem(char *s, int *i)
{
  int type;
  struct dataAttributeType *dAT=0;
  
  type = whichEntry(s, i);
  switch(type) {
  case 3: /* Nominal */
    dAT = generateNominalAttribute(s, *i);
    break;
  case 4: /* Integral */
    dAT = generateIntegerAttribute(s, *i);
    break;
  case 5: /* Numerical */
    assert((dAT = (struct dataAttributeType *)calloc(1, sizeof(struct dataAttributeType)))!=NULL);
    dAT->type = 4;
    break;
  case 6: /* Real */
    dAT = generateRealAttribute(s, *i);
    break;
  default: /* not supported / unknown */
    printf("Format not supported!!! %d %s\n", type, s);
    return 0;
    break;
  }
  return dAT;
}

/**
 * Creates and integer attribute type especially reading in the boundaries.
 */
struct dataAttributeType * generateIntegerAttribute(char *s, int i)
{
  int begin, end;
  char name[100];
  struct dataAttributeType * dAT;
  
  assert((dAT = (struct dataAttributeType *)calloc(1, sizeof(struct dataAttributeType)))!=NULL);
  dAT->type = 1;
  
  for(; s[i]!='\0'; i++)
    if(s[i]!=' ' && s[i]!='\t')
      break;
  if(s[i]!='[') {
    if(s[i]!='\0'){
      printf("Integral ranges are enclosed with []!\n");
      free(dAT);
      return 0;
    }else{
      /* no ranges specified -> generating them online! */
      dAT->lowerBound=0;
      dAT->upperBound=-1;
      return dAT;
    }
  }

  /* read lower bound */
  i++;
  begin=i;
  for( ; s[i]!=',' && s[i]!=':' && s[i]!=']' && s[i]!='\0'; i++) {
    ;/* find end of name */
  }
  end=i;
  strncpy(name, &(s[begin]), end-begin);
  name[end-begin]='\0';
  dAT->lowerBound=getIntValue(name);

  /* read upper bound */
  i++;
  begin=i;
  for( ; s[i]!=',' && s[i]!=':' && s[i]!=']' && s[i]!='\0'; i++) {
    ;/* find end of name */
  }
  end=i;
  strncpy(name, &(s[begin]), end-begin);
  name[end-begin]='\0';
  dAT->upperBound=getIntValue(name);

  if(dAT->lowerBound >= dAT->upperBound) {
    printf("Could not read bounds correctly! Lower bound larger than upper bound %d-%d!?\n", dAT->lowerBound, dAT->upperBound);
    free(dAT);
    return 0;
  }  
  return dAT;
}

/**
 * Creates a real attribute type especially reading in the boundaries.
 */
struct dataAttributeType * generateRealAttribute(char *s, int i)
{
  int begin, end;
  char name[100];
  struct dataAttributeType * dAT;

  assert((dAT = (struct dataAttributeType *)calloc(1, sizeof(struct dataAttributeType)))!=NULL);
  dAT->type = 2;
  
  for(; s[i]!='\0'; i++)
    if(s[i]!=' ' && s[i]!='\t')
      break;
  if(s[i]!='[') {
    if(s[i]!='\0'){
      printf("Real ranges are enclosed with []!\n");
      free(dAT);
      return 0;
    }else{
      /* no ranges specified -> generating them online! */
      dAT->lowerRealBound=0;
      dAT->upperRealBound=-1;
      return dAT;
    }
  }

  /* read lower bound */
  i++;
  begin=i;
  for( ; s[i]!=',' && s[i]!=':' && s[i]!=']' && s[i]!='\0'; i++) {
    ;/* find end of name */
  }
  end=i;
  strncpy(name, &(s[begin]), end-begin);
  name[end-begin]='\0';
  dAT->lowerRealBound=getDoubleValue(name);

  /* read upper bound */
  i++;
  begin=i;
  for( ; s[i]!=',' && s[i]!=':' && s[i]!=']' && s[i]!='\0'; i++) {
    ;/* find end of name */
  }
  end=i;
  strncpy(name, &(s[begin]), end-begin);
  name[end-begin]='\0';
  dAT->upperRealBound=getDoubleValue(name);

  if(dAT->lowerRealBound >= dAT->upperRealBound) {
    printf("Could not read bounds correctly! Lower bound larger than upper bound!?\n");
    free(dAT);
    return 0;
  }
  return dAT;
}

/**
 * Creates a nominal attribute type reading in the nominal names
 */
struct dataAttributeType * generateNominalAttribute(char *s, int i)
{
  int begin, end, j;
  char *name;
  char **help;
  struct dataAttributeType *dAT;

  for(; s[i]!='\0'; i++)
    if(s[i]!=' ' && s[i]!='\t')
      break;
  if(s[i]!='{') {
    printf("Nominals are enclosed with {}!\n");
    return 0;
  }
  assert((dAT = (struct dataAttributeType *)calloc(1, sizeof(struct dataAttributeType)))!=NULL);
  dAT->type = 3;
  dAT->lowerBound=0;
  dAT->upperBound=-1;
  while(s[i]!='}' && s[i]!='\0') {
    i++;
    for(; s[i]!='\0'; i++)
      if(s[i]!=',' && s[i]!=' ' && s[i]!='\t')
	break;
    begin=i;
    for( ; s[i]!=',' && s[i]!='}' && s[i]!='\0'; i++) {
      ;/* find end of name */
    }
    if(i == begin) {
      printf("Empty Entries are not allowed for nominals!\n:");
      return 0;
    }
    end=i;
    while(s[end-1]==' ' || s[end-1]=='\t')
      end--; /* get rid of ending spaces */
    assert((name = (char *)calloc((end-begin)+1, sizeof(char)))!=NULL);
    strncpy(name, &(s[begin]), end-begin);
    name[end-begin]='\0';
    /* add to the nominals */
    assert((help = (char **)calloc((dAT->upperBound)+2, sizeof(char *)))!=NULL);
    for(j=0; j<dAT->upperBound+1; j++) {
      help[j] = (dAT->nominals)[j];
    }
    help[j] = name;
    if(dAT->upperBound >= 0)
      free(dAT->nominals);
    dAT->nominals = help;
    (dAT->upperBound)++;
    for( ; s[i]==' ' || s[i]=='\t'; i++) {
      ;/* find next komma / end of entry */
    }
  }
  printf("Done reading %d nominal values\n", (dAT->upperBound)+1);
  return dAT;
}

/**
 * Determines which of the key-words is used.
 */
int whichEntry(char *s, int *pos)
{
  int i, type, j, match;

  char *(a[16]);
  assert((a[0] = (char *)(calloc(9, sizeof(char))))!=NULL);
  assert((a[1] = (char *)(calloc(9, sizeof(char))))!=NULL);
  assert((a[2] = (char *)(calloc(10, sizeof(char))))!=NULL);
  assert((a[3] = (char *)(calloc(10, sizeof(char))))!=NULL);
  assert((a[4] = (char *)(calloc(5, sizeof(char))))!=NULL);
  assert((a[5] = (char *)(calloc(5, sizeof(char))))!=NULL);
  assert((a[6] = (char *)(calloc(8, sizeof(char))))!=NULL);
  assert((a[7] = (char *)(calloc(8, sizeof(char))))!=NULL);
  assert((a[8] = (char *)(calloc(8, sizeof(char))))!=NULL);
  assert((a[9] = (char *)(calloc(8, sizeof(char))))!=NULL);
  assert((a[10] = (char *)(calloc(10, sizeof(char))))!=NULL);
  assert((a[11] = (char *)(calloc(10, sizeof(char))))!=NULL);
  assert((a[12] = (char *)(calloc(5, sizeof(char))))!=NULL);
  assert((a[13] = (char *)(calloc(5, sizeof(char))))!=NULL);
  assert((a[14] = (char *)(calloc(6, sizeof(char))))!=NULL);
  assert((a[15] = (char *)(calloc(6, sizeof(char))))!=NULL);

  strcpy(a[0],"relation");
  strcpy(a[1],"RELATION");
  strcpy(a[2],"attribute");
  strcpy(a[3],"ATTRIBUTE");
  strcpy(a[4],"data");
  strcpy(a[5],"DATA");
  strcpy(a[6],"nominal");
  strcpy(a[7],"NOMINAL");
  strcpy(a[8],"integer");
  strcpy(a[9],"INTEGER");
  strcpy(a[10],"numerical");
  strcpy(a[11],"NUMERICAL");
  strcpy(a[12],"real");
  strcpy(a[13],"REAL");
  strcpy(a[14],"class");
  strcpy(a[15],"CLASS");
  
  i = *pos;
  for( ; s[i]!='\0'; i++)
    if(s[i]!=' ' && s[i]!='\t')
      break;
  if(s[i] == '@'){
    i++;
  }else if(i==0) {
    printf("Parse Error - entry does not start with an '@'%d:\n%s\n",*pos,s);
    return -1;
  }
  type = -1;
  match=0;
  j=0;

  for( ; s[i]!='\0' && match==0; i++) {
    if(type==-1) {
      if(s[i]==a[0][0] || s[i]==a[1][0]) {
	i++;
	j++;
	if(s[i]==a[0][j] || s[i]==a[1][j]) {
	  i++;
	  j++;
	  if(s[i]==a[0][j] || s[i]==a[1][j]) {
	    type=0;
	  }else if(s[i]==a[12][j] || s[i]==a[13][j]) {
	    type=6;
	  }else{
	    printf("Parse Error - entry starts with unknown symbol combination after the '@re'\n");
	    return -1;
	  }
	}else{
	  printf("Parse Error - entry starts with unknown symbol combination after the '@r'\n");
	  return -1;
	}
      }else if(s[i]==a[2][0] || s[i]==a[3][0])
	type=1;
      else if(s[i]==a[4][0] || s[i]==a[5][0])
	type=2;
      else if(s[i]==a[6][0] || s[i]==a[7][0]) {
	i++;
	j++;
	if(s[i]==a[6][j] || s[i]==a[7][j]) 
	  type=3;
	else if(s[i]==a[10][j] || s[i]==a[11][j])
	  type=5;
	else{
	  printf("Parse Error - entry starts with unknown symbol combination after the '@':%s\n",&(s[i]));
	  return -1;
	}
      }else if(s[i]==a[8][0] || s[i]==a[9][0])
	type=4;
      else if(s[i]==a[14][0] || s[i]==a[15][0])
	type=7;
      else if(s[i]=='{') {/* nominal starts simply with curly breakets */
	type=3;
	break;
      }else{
	printf("Parse Error - entry starts with an unknown symbol after the '@':%s\n",&(s[i]));
	return -1;
      }
      j++;
    }else{
      if(s[i] != a[type*2][j] && s[i] != a[(type*2)+1][j]) {
	printf("Parse Error - key word is unknown \n");
	return -1;
      }
      j++;
      if(a[type*2][j]=='\0')
	match=1;
    }
  }

  for(j=0; j<12; j++) 
    free(a[j]);

  if(type==7) /* class is always a nominal */
    type=3;
  *pos = i;

  if(type==-1) {
    printf("Type is %d=%s\n", type, s);
  }
  return type;
}

/**
 * Determines the name in the string (possibly in quotes). 
 * *pos is an index that points to the beginning of the name in the string s. 
 * @return returns the name including the space allocation for it.
 */
char *getName(char *s, int *pos)
{ 
  int i, begin, end;
  int quote;
  char *name;

  i = *pos;
  quote=0;
  for( ; s[i]!='\0'; i++)
    if(s[i]!=' ' && s[i]!='\t')
      break;
  if(s[i]=='"' || s[i]=='\'') {
    quote=1;
    i++;
  }
  begin=i;
  for( ; s[i]!='\0'; i++) {
    if((s[i]==' '|| s[i]=='\t') && quote==0) {
      break;
    }else if( (s[i]=='"' || s[i]=='\'') && quote) {
      break;
    }
  }
  if(s[i]=='\0' && quote) {
    printf("Quoted Name did not end: %s\n", &(s[*pos]));
    return 0;
  }
  end=i;
  if(quote) /* set i to next empty position */
    i++;
  assert((name = (char *)calloc((end-begin)+1, sizeof(char)))!=NULL);
  strncpy(name, &(s[begin]), end-begin);
  name[end-begin]='\0';

  *pos = i;
  return name;
}

/**
 * Adds the attribute with name "name" and dataType "att" to the attribute list attNames, attTypes.
 */
void addAttribute(char *name, struct dataAttributeType *att)
{
  char **help;
  struct dataAttributeType *attHelp;
  int i;
  
  assert((help = (char **)calloc(attNum+1, sizeof(char *)))!=NULL);
  assert((attHelp = (struct dataAttributeType *)calloc(attNum+1, sizeof(struct dataAttributeType)))!=NULL);
  for(i=0; i<attNum; i++) { 
    help[i] = attNames[i];
    attHelp[i] = attTypes[i];
  }
  help[i] = name;
  attHelp[i] = *att;
  free(attNames);
  free(attTypes);
  free(att);
  attNames = help;
  attTypes = attHelp;
  attNum++;
}

/**
 * Prepares the data for stratified 10-folded cross validation
 */
int do10FoldedCrossValidation(long seed)
{
  int i, j, *class, **classes, cpos, pos, entNumCurrent, fold, clnum, type;
  struct dataAttribute **help, *hh;
  long saveSeed;

  saveSeed = getSeed();
  setSeed(seed);

  if(entNumTotal<10) {
    return 0;
  }
  assert((class = (int *)calloc(getNumberOfActions(), sizeof(int)))!=NULL);
  cpos = attNum-1;
  for(i=0; i<entNumTotal; i++) {
    (class[ data[i][cpos].integer]) ++;
  }
  printf("Total=%d, Class distribution: ", entNumTotal);
  for(i=0; i<getNumberOfActions(); i++) {
    printf("%d ",class[i]); 
  }
  printf("\n");

  /* determine how many guys of each class each fold should have */
  assert((classes = (int **)calloc(10, sizeof(int*)))!=NULL);
  for(i=0; i<10; i++) {
    printf("Fold %d: ", i);
    assert((classes[i] = (int *)calloc(getNumberOfActions(), sizeof(int)))!=NULL);
    for(j=0; j<getNumberOfActions(); j++) {
      classes[i][j] = (int)(class[j]/10);
      if(class[j]%10 > i) {
	(classes[i][j])++;
      }
      printf("%d ",classes[i][j]); 
    }
    printf("\n");
  }
  free(class);

  /* now determine the folds... */
  assert((arffFoldEnds = (int *)calloc(10, sizeof(int)))!=NULL);
  help = data;
  assert((data = (struct dataAttribute **)calloc(entArraySize, sizeof(struct dataAttribute*)))!=NULL);
  entNumCurrent = entNumTotal;
  fold=0;
  clnum=0;
  for(j=0; j<getNumberOfActions(); j++) {
    if(classes[fold][j]>0)
      clnum ++; /* determines the number of represented classes in this fold */
  }
  for(i=0; i<entNumTotal; i++) {
    /* determine next problem instance with an available class*/
    do{
      pos = (int)(urand() * entNumCurrent);
      type = help[pos][attNum-1].integer;
    }while(classes[fold][type]<=0);
    (classes[fold][type])--;
    if(classes[fold][type] == 0) {
      clnum--;
    }

    data[i] = help[pos];
    entNumCurrent--;
    hh = help[pos];
    help[pos] = help[entNumCurrent];
    help[entNumCurrent] = hh;

    if(clnum==0) {
      (arffFoldEnds[fold])=i+1;    
      fold++;
      if(fold<10) {
	for(j=0; j<getNumberOfActions(); j++) {
	  if(classes[fold][j]>0)
	    clnum ++; /* determines the number of represented classes in this fold */
	}
      }else{
	if(entNumTotal != i+1) {
	  printf("Not all instances were assigned correctly!\n");
	} 
      }
    }
  }
  for(i=0; i<10; i++) {
    free(classes[i]);
  }
  free(classes); /* note that the "classes" instances are still in use! */
  free(help);

  arffDo10Folded = 1;
  arffExpNumber = 0;
  arffIsTesting = 0;

  setSeed(saveSeed);
  return 1;
}


/**
 * Sets the system to test mode and back to normal mode dependent on doIt.
 * @return Returns the size of the learning set / test set
 */
int doTesting(int doIt)
{
  int length=0;

  if(doIt==0 && arffIsTesting) {
    arffIsTesting=0;
    arffExpNumber++;
    if(arffExpNumber>9){
      arffExpNumber=0;
      /* resample the folds ! */
      /* do10FoldedCrossValidation(); */
    }
    length = entNumTotal - arffFoldEnds[arffExpNumber];
    if(arffExpNumber>0)
      length += arffFoldEnds[arffExpNumber-1];
  }else if(doIt && arffIsTesting==0) {
    arffIsTesting=1;
    length = arffFoldEnds[arffExpNumber];
    if(arffExpNumber>0)
      length -= arffFoldEnds[arffExpNumber-1];
  }
  arffTestProblemNr=0;
  if(!arffDo10Folded) {
    return entNumTotal;
  }
  return length;
}

/** 
 * Specifies the next test problem in state. 
 * Returns if there is another test problem.
 * If there is none, returns zero and resets the test problems. 
 * If not in test mode, returns zero and does nothing
 */
int nextTestState(struct attributes *state)
{
  int length;
  if(! arffIsTesting) {
    /* if not in test mode, do nothing and return zero */
    return 0;
  }
  if(!arffDo10Folded) {
    length = entNumTotal;
  }else{
    length =  arffFoldEnds[arffExpNumber];
    if(arffExpNumber>0)
      length -= arffFoldEnds[arffExpNumber-1];
  }
  if(arffTestProblemNr == length) {
    /* if last test problem was reached, reset to the first and return 0 */
    arffTestProblemNr = 0;
    return 0;
  }
  currentChoice = arffTestProblemNr;
  if(arffDo10Folded && arffExpNumber>0) {
    currentChoice += arffFoldEnds[arffExpNumber-1];
  }
  setArffState(state);
  /*
   * fprintf(stdout, "%d: ", arffTestProblemNr);
   * fprintState(stdout, state);
   * fprintf(stdout, " = %d;", data[currentChoice][attNum-1].integer);
   */
  arffTestProblemNr++;
  return 1;
}

/**
 * Sets state to the chosen problem instance (specified in currentChoice)
 */
void setArffState(struct attributes *state)
{
  int i;

  for(i=0; i<attNum-1; i++) {/* last one is the class */
    state[i].type = data[currentChoice][i].type;
    switch(state[i].type) {
    case 0: /* boolean */
      state[i].bool = data[currentChoice][i].bool;
      break;
    case 1: /* integer */
      state[i].integer = data[currentChoice][i].integer;
      break;
    case 2: /* real */
      state[i].real = data[currentChoice][i].real;
      break;
    case 3: /* nominal -> handled like an integer right now ! */
      state[i].type = 1; 
      state[i].integer = data[currentChoice][i].integer;
      break;
    case 4: /* numerical -> is like real*/
      state[i].type = 2;
      state[i].real = data[currentChoice][i].real;
      break;
    }
  }
}
