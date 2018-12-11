/*
/       (XCS-C 1.1)
/	------------------------------------
/	Learning Classifier System based on accuracy
/
/     by Martin Butz
/     University of Wuerzburg / University of Illinois at Urbana/Champaign
/     butz@illigal.ge.uiuc.edu
/     Last modified: 09-04-2003
/
/     Boolean Functions: constant, random, parity, multiplexer, biased multiplexer, catentated multiplexer
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "intValuedFktEnv.h"
#include "xcsMacros.h"

/* the current properties of this boolean function */
struct integerEnv *integerFuncProps = 0;
double getMADMPOutcomes(int i, int pos, char *conArray, int class, double average);
double getAveragedMPOutcomes(int i, int pos, char *conArray, int class);
double getAveragedCO0Outcomes(char *conArray, int class);
double getAveragedCO1Outcomes(int i, char *conArray, int class);
double getMADCO1Outcomes(int i, char *conArray, int class, double average);
int nicheState=0;

/**
 * Returns that this is a single step environment.
 */ 
int isMultiStep()
{
  return 0;
}

/**
 * Returns the lenght of a problem instance.
 */
int getConditionLength()
{
  return integerFuncProps->conditionLength;
}

/**
 * Returns the type (int, boolean, integer) for the specified type.
 */
int getAttributeType(int att)
{
  return getGeneralType(); /* this is the normal Boolean type */
}

/**
 * Returns the upper bound of an attribute.
 */
double getUpperBound(int att)
{
  return 10.; /* integer case we go from 0 to 1 */
}

/**
 * Returns the lower bound of an attribute.
 */
double getLowerBound(int att)
{
  return 1; /* integer case we go from 0 to 1 */
}

/**
 * Returns the payment range of this problem (i.e. the maximum payoff).
 */
int getPaymentRange()
{
  return integerFuncProps->paymentRange;
}

/**
 * Returns the number of possible actions.
 */
int getNumberOfActions()
{
  if(integerFuncProps->concatenatedMultiplexer==1)
    return (int)(1<<(int)((integerFuncProps->conditionLength)/(integerFuncProps->multiplexerBits + (1<<integerFuncProps->multiplexerBits))));
  return 2;
}

/**
 * Resets the state to a new problem instance.
 */ 
void resetState(struct attributes *state)
{
  int i, redo=1;
  int action=0, correct=0;
  nicheState=0;
  /* generate a random state */
  while(redo) {
    for(i=0; i < integerFuncProps->conditionLength; i++) {
      state[i].integer = getLowerBound(i) + (int)( urand() * (1.+ getUpperBound(i)-getLowerBound(i)));
      state[i].type = getGeneralType();
    }
    if(integerFuncProps->samplingBias == 0.5) {
      redo = 0;
    }else{
      doAction(state, action, &correct);
      /* accept a string with result '0' with probability samplingBias */
      redo=0;
      if(correct==1) {
	if(urand() < integerFuncProps->samplingBias)
	  redo = 1;
      }else{
	if(urand() > integerFuncProps->samplingBias)
	  redo = 1;
      }
    }
  }
}

/** 
 * Execute one function classification (feedback in 'correct' and the reward value)
 */ 
double doAction(struct attributes *ss, int act,int *correct)
{
  double value;
  char *state;
  int i;
  
  assert((state = calloc(getConditionLength()+1, sizeof(char)))!=NULL);
  for(i=0; i<getConditionLength(); i++) {
    if(ss[i].integer <= getLowerBound(i) + (getUpperBound(i)-getLowerBound(i))/ 2)
      state[i] = '0';
    else
      state[i] = '1';
  }
  
  /* return the perceived reward */
  if(integerFuncProps->constantFunction) {
    *correct = 1;
    value = (double)integerFuncProps->paymentRange;
  }else if(integerFuncProps->randomFunction)
    value = doRandomAction(state,act,correct);
  else if(integerFuncProps->parityFunction)
    value = doParityAction(state,act,correct);
  else if(integerFuncProps->multiplexerFunction)
    value = doMPAction(state,act,correct);
  else if(integerFuncProps->concatenatedMultiplexer)
    value = doConcatenatedMultiplexerAction(state, act, correct);
  else if(integerFuncProps->biasedMultiplexer)
    value = doBiasedMultiplexerAction(state,act,correct);
  else if(integerFuncProps->countOnesFunction)
    value = doCountOnesAction(state, act, correct);
  else if(integerFuncProps->carryFunction)
    value = doCarryAction(state, act, correct);
  else if(integerFuncProps->classifierFunction)
    value = doClassifierAction(state, act, correct);
  else{
    value = 0;
    printf("Some function should have been chosen in env.h or set in the input file!\n");
  }

  if(integerFuncProps->addNoiseToAction>0) {
    if(integerFuncProps->addNoiseToAction<4) { /* Add Gaussian noise to outcome value */
      if(  integerFuncProps->addNoiseToAction==1 || (integerFuncProps->addNoiseToAction==2 && act==0) || (integerFuncProps->addNoiseToAction==3 && act==1))
	value += (integerFuncProps->actionNoiseMu) + nrand() * (integerFuncProps->actionNoiseSigma);
    }else{ /* Alternate outcome with a certain probabilitiy possibly action dependent */
      if(  integerFuncProps->addNoiseToAction<=5 || (integerFuncProps->addNoiseToAction<=6 && act==0) || (integerFuncProps->addNoiseToAction>6 && integerFuncProps->addNoiseToAction<=7 && act==1)) {
	if(urand() < integerFuncProps->addNoiseToAction-(double)(int)(integerFuncProps->addNoiseToAction)) {
	  if(value == integerFuncProps->paymentRange)
	    value = 0;
	  else
	    value = integerFuncProps->paymentRange;
	}
      }
    }
  }

  free(state);
  return value;
}

/**
 * Random Function
 */
double doRandomAction(char *state, int act, int *correct)
{
  *correct = ((double) rand() / (RAND_MAX+1.0)) *2;
  return (double)((int)(((double) rand() / (RAND_MAX+1.0)) *2) * (integerFuncProps->paymentRange));
}

double doClassifierAction(char *state, int act, int *correct)
{
  if(act==0) {
    *correct=0;
    return 0;
  }
  *correct=1;
  return 1000;
}

double doCarryAction(char *state, int act, int *correct)
{
  int i,j;
  int carry=0;

  j=integerFuncProps->carrySize;
  for(i=0; i<integerFuncProps->carrySize; i++,j++) {
    if(state[i]=='0' && state[j]=='0') {
      break;
    }else if(state[i]=='1' && state[j]=='1') {
      carry=1;
      break;
    }
  }
  
  if(act==carry) {
    *correct=1;
    return integerFuncProps->paymentRange;
  }
  *correct=0;
  return 0;
}

/**
 * Count Ones Function
 */
double doCountOnesAction(char *state, int act, int *correct)
{
  int i,j;
  for(i=0, j=0; i<integerFuncProps->countOnesSize; i++){
    if(state[i]=='1')/* count the number of ones */
      j++;
  }
  switch(integerFuncProps->countOnesType){
  case 0: /* the number of ones determines the outcome */
    if(j*2 >= integerFuncProps->countOnesSize){ /* this is the action one */
      if(act==1){
	*correct = 1;
	return integerFuncProps->paymentRange;
      }else{
	*correct = 0;
	return 0;
      }
    }else { /* If equal or less, the action should be zero! */
      if(act==0){
	*correct = 1;
	return integerFuncProps->paymentRange;
      }else{
	*correct = 0;
	return 0;
      }
    }
    break;
  case 1: /* The number of ones determines the payoff level returned */
    if(j*2 > integerFuncProps->countOnesSize){ /* this is the action one */
      if(act==1) {
	*correct = 1;
	return (integerFuncProps->paymentRange) * j / (integerFuncProps->countOnesSize);
      }else{
	*correct = 0;
	return (integerFuncProps->paymentRange) * ((integerFuncProps->countOnesSize)-j) / (integerFuncProps->countOnesSize);
      }
    }else if(j*2 == integerFuncProps->countOnesSize) { /* 50/50 here - with this it is not possible to distinguish between action - so either action is correct!*/
      *correct = 1;
      return (integerFuncProps->paymentRange)/2;
    }else{
      if(act==0){
	*correct = 1;
	return (integerFuncProps->paymentRange)*((integerFuncProps->countOnesSize)-j) / (integerFuncProps->countOnesSize);
      }else{
	*correct = 0;
	return (integerFuncProps->paymentRange) * j / (integerFuncProps->countOnesSize);
      }
    }
    break;
  default:
    printf("Incorrect Count Ones Type\n");
    break;
  }
  return 0;
}

/**
 * Hidden Parity Function.
 */
double doParityAction(char *state, int act, int *correct)
{
  int i, sum=0;
  for(i=0; i<integerFuncProps->paritySize; i++){
    if(state[i]=='1')
      sum++;
  }
  if(sum%2==0)
    if(act==0){
      *correct =1;
      return integerFuncProps->paymentRange;
    }else{
      *correct =0;
      return 0.;
    }
  else
    if(act==0){
      *correct = 0;
      return 0.;
    }else{
      *correct = 1;
      return integerFuncProps->paymentRange;
    }
}

/**
 * Multiplexer Function
 */
double doMPAction(char *state, int act, int *correct)
{
  int place=integerFuncProps->multiplexerBits;
  int i;
  double reward;
 
  /* get the place of the by the first index bits referenced spot */
  for(i=0,place=integerFuncProps->multiplexerBits; i<integerFuncProps->multiplexerBits; i++) {
    if(state[i]=='1')
      place += (int)(1<<((integerFuncProps->multiplexerBits)-1-i));
  }
  /* determine the corresponding reward and set 'correct' */
  if((act==1 && state[place]=='1') || (act==0 && state[place]=='0')) {
    /* the correct classification was chosen */
    *correct=1;
    if(integerFuncProps->payoffLandscape)
      reward= 300.+(double)(((place-(integerFuncProps->multiplexerBits))*200)+100*(int)(state[place]-'0'));
    else
      reward = integerFuncProps->paymentRange;
  }else{
    /* the incorrect classification was chosen */
    *correct=0;
    if(integerFuncProps->payoffLandscape)
      reward= 0.+(double)(((place - (integerFuncProps->multiplexerBits))*200)+100*(int)(state[place]-'0'));
    else
      reward = 0;
  }
  return reward;
}

/**
 * This is the Biased Multiplexer Function!
 */
double doBiasedMultiplexerAction(char *state, int act, int *correct)
{
  int i,place,increment,zeroone=0,placelocal=0;
  char action='\0';

  for(i=0,place=0; i<integerFuncProps->bmpX; i++){
    if(state[i]=='1')
      place += (int)(1<<(integerFuncProps->bmpX-i-1));
  }
  if(place >= ((int)(1<<(integerFuncProps->bmpX)))/2)
    zeroone=1;
  increment=((int)(1<<(integerFuncProps->bmpY))) - 1 + integerFuncProps->bmpY;
  for(placelocal=0, i=0; i< integerFuncProps->bmpY; i++){
    if(state[integerFuncProps->bmpX + place * increment + i]=='1')
      placelocal += (int)(1<<(integerFuncProps->bmpY-i-1));
  }
  if(zeroone==0){
    if(placelocal==0){
      action='0';
    }else{
      placelocal--;
      action=state[integerFuncProps->bmpX + place*increment + integerFuncProps->bmpY + placelocal];
    }
  }else{
    if(placelocal == ((int)(1<<integerFuncProps->bmpY))-1){
      action='1';
    }else{
      action=state[integerFuncProps->bmpX + place*increment + integerFuncProps->bmpY + placelocal];
    }
  }
  /* printf("%s -> %c\n",state,action);*/
 if((action=='1'&& act==1) || (action=='0' && act==0)){
    *correct=1;
    return integerFuncProps->paymentRange;
  }else{
    *correct=0;
    return 0;
  }
}

/**
 * Concatenated Multiplexer Function
 */
double doConcatenatedMultiplexerAction(char *state, int act, int *correct)
{
  int place=integerFuncProps->multiplexerBits;
  int nrMPs = (int)((integerFuncProps->conditionLength)/(integerFuncProps->multiplexerBits + (1<<integerFuncProps->multiplexerBits)));
  int length = integerFuncProps->multiplexerBits + (1<<integerFuncProps->multiplexerBits);
  int i,j, nOne;
  double reward;
 
  *correct=1;
  reward=0;
  nOne=0;
  for(i=0; i<nrMPs; i++) {
    /* get the place of the spot referenced by the first index bits */
    for(j=0, place=length*i+integerFuncProps->multiplexerBits; j < integerFuncProps->multiplexerBits; j++) {
      if(state[length*i+j]=='1')
	place += (int)(1<<((integerFuncProps->multiplexerBits)-1-j));
    }
    /* determine the corresponding reward and set 'correct' */
    if(integerFuncProps->concatenatedMultiplexer==1) {
      if((act==1 && state[place]=='1') || (act==0 && state[place]=='0')) {
	/* the right action was chosen */
	if(integerFuncProps->payoffLandscape)
	  reward += 1000;
      }else{
	/* the wrong action was chosen */
	*correct=0;
	if(!integerFuncProps->payoffLandscape)
	  break; /* if one is wrong and no payoff Landscape, all is wrong */
      }
    }else{
      if(state[place]=='1') {
	nOne++;
      }
    }
  }
  if(integerFuncProps->concatenatedMultiplexer==2) {
    if((nOne>1 && act==1) || (nOne<2 && act==0)) {/*type with only one action*/
      /*printf("Is correct %s - %s: %d\n",state, act, nOne);*/
      *correct = 1;
      reward = 1000;
    }else{
      *correct = 0;
      reward = 0;
    }
    if(integerFuncProps->payoffLandscape) {
      if(act==1)
	reward = 1000 * nOne;
      else
	reward = 1000 * (nrMPs-nOne);
    }
  }else{
    if(*correct && !integerFuncProps->payoffLandscape)
      reward = 1000;
    /* printf("%s with act %s was assigned %d with reward %f\n",state, act, *correct, reward);*/
  }
  return reward;
}




int getNextNiche(struct attributes *state)
{
  int num, i, j, k, l, move;
  
  if(integerFuncProps->multiplexerFunction) {
    /* Multiplexer Function */
    num = (int)(1<<(integerFuncProps->multiplexerBits + 1));
    if(num <= nicheState) /* no more niches */
      return 0;
    
    /* more niches to come... */
    j=nicheState;
    for(i=0; i<integerFuncProps->multiplexerBits; i++) {
      (state[i]).type = 2;
      if( j >= (int)(1<<(integerFuncProps->multiplexerBits-i))) {
	(state[i]).integer=1.;
	j -= (int)(1<<(integerFuncProps->multiplexerBits-i));
      }else{
	(state[i]).integer=0.;
      }
    }
    for(j=0; j<(1<<(integerFuncProps->multiplexerBits)); j++) {
      (state[integerFuncProps->multiplexerBits+j]).type = 0;
      (state[integerFuncProps->multiplexerBits+j]).bool = DONT_CARE;
    }
    j = nicheState/2;
    (state[integerFuncProps->multiplexerBits+j]).type = 2;
    if(nicheState%2==0)
      (state[integerFuncProps->multiplexerBits+j]).integer = getLowerBound(integerFuncProps->multiplexerBits+j);
    else
      (state[integerFuncProps->multiplexerBits+j]).integer = getUpperBound(integerFuncProps->multiplexerBits+j);
    nicheState++;
    return 1;
  }else if(integerFuncProps->countOnesFunction) {
    if(integerFuncProps->countOnesType==1) {
      /* Count ones problem type 1 */
      num = (int)(1<<integerFuncProps->countOnesSize);
      if(num <= nicheState)
	return 0;
      
      /* more niches to come... */
      for(i=0; i<integerFuncProps->conditionLength; i++) {
	(state[i]).type = 0;
	(state[i]).bool=DONT_CARE;
      }
      j=nicheState;
      for(i=0; i<integerFuncProps->countOnesSize; i++) {
	if( j >= (int)(1<<(integerFuncProps->countOnesSize-i-1))) {
	  (state[i]).type = 2;
	  (state[i]).integer=getUpperBound(i);
	  j -= (int)(1<<(integerFuncProps->multiplexerBits-i-1));
	}else{
	  (state[i]).type = 2;
	  (state[i]).integer=getLowerBound(i);
	}
      }
      nicheState++;
      return 1;
    }else{
      /* Count ones problem type 0 */
      num = 2 * NChooseK(integerFuncProps->countOnesSize, (integerFuncProps->countOnesSize-1)/2 );
      if(num <= nicheState)
	return 0;

      /* MORE NICHES TO COME */
      for(i=0; i<integerFuncProps->conditionLength; i++) {
	(state[i]).type = 0;
	(state[i]).integer=DONT_CARE;
      }
      /* zero or one niche */
      if(nicheState < num/2) {
	(state[0]).type = 2;
	(state[0]).integer=getLowerBound(0);
	j=nicheState;
      }else{
	(state[0]).type = 2;
	(state[0]).integer=getUpperBound(0);
	j=nicheState - num/2;
      }
      /* need to generate more than half to be correct */
      for(i=1; i<(integerFuncProps->countOnesSize+1)/2; i++) {
	(state[i]).type = 2;
	(state[i]).integer=(state[i-1]).integer;
      }

      /* iteratively generate actual test niche */
      for(k=0; k<j; k++) {
	if((state[integerFuncProps->countOnesSize-1]).type == 0) {
	  /* just move last entry */
	  for( i = 1; i<integerFuncProps->countOnesSize ; i++) {
	    if((state[integerFuncProps->countOnesSize-1-i]).type != 0){
	      (state[integerFuncProps->countOnesSize-i]).type = 2;
	      (state[integerFuncProps->countOnesSize-i]).integer = (state[integerFuncProps->countOnesSize-1-i]).integer;
	      (state[integerFuncProps->countOnesSize-1-i]).type = 0;
	      (state[integerFuncProps->countOnesSize-1-i]).integer = DONT_CARE;
	      break; 
	    }
	  }
	}else{
	  /* move chunk to next state */
	  (state[integerFuncProps->countOnesSize-1]).type = 0;
	  (state[integerFuncProps->countOnesSize-1]).bool = DONT_CARE;
	  /* detect chunk to be moved */
	  for( i = 1; i<integerFuncProps->countOnesSize; i++) {
	    if( (state[integerFuncProps->countOnesSize-1-i]).type == 0) {
	      move = i;
	      if(move == (integerFuncProps->countOnesSize+1)/2 ){
		printf("Seems to be done!?\n");
		return 0;
	      }
	      break;
	    }else{
	      (state[integerFuncProps->countOnesSize-1-i]).type = 0;
	      (state[integerFuncProps->countOnesSize-1-i]).bool = DONT_CARE;
	    }
	  }
	  /* move specifies the number of guys that will be moved back */
	  /* look for place where to move it */
	  for(i++; i<integerFuncProps->countOnesSize; i++) {
	    if((state[integerFuncProps->countOnesSize-1-i]).type != 0) {
	      /* found it */
	      (state[integerFuncProps->countOnesSize-i]).type = 2;
	      (state[integerFuncProps->countOnesSize-i]).integer=(state[integerFuncProps->countOnesSize-1-i]).integer;
	      (state[integerFuncProps->countOnesSize-1-i]).type = 0;
	      (state[integerFuncProps->countOnesSize-1-i]).bool = DONT_CARE;
	      for(l=0; l<move; l++) {
		(state[integerFuncProps->countOnesSize-i+1+l]).type = 2;
		(state[integerFuncProps->countOnesSize-i+1+l]).integer = (state[integerFuncProps->countOnesSize-i+l]).integer;
	      }
	      break;
	    }
	  }
	}
	/* next iteration */
      }
      nicheState++;
      return 1;
    }
  }else if(integerFuncProps->carryFunction) {
    /* MORE NICHES TO COME */
    for(i=0; i<integerFuncProps->conditionLength; i++) {
      (state[i]).type = 0;
      (state[i]).bool = DONT_CARE;
    }
    i=0;
    j=integerFuncProps->carrySize;
    
    if(nicheState+1 < (int)(1<<(integerFuncProps->carrySize))) {
      (state[i]).type = 2;
      (state[i]).integer=getUpperBound(i);
      (state[j]).type = 2;
      (state[j]).integer=getUpperBound(j);
      for(k=0; k<nicheState; k++) {
	(state[i]).type = 2;
	(state[i]).integer=getUpperBound(i);
	(state[j]).type = 0;
	(state[j]).bool = DONT_CARE;
	if(i+1 < integerFuncProps->carrySize) {
	  /* set the next position to both '1' so that there is a carry */
	  i++;
	  j++;
	  (state[i]).type = 2;
	  (state[i]).integer=getUpperBound(i);
	  (state[j]).type = 2;
	  (state[j]).integer=getUpperBound(j);
	}else{
	  (state[i]).type = 0;
	  (state[i]).bool = DONT_CARE;
	  (state[j]).type = 0;
	  (state[j]).bool = DONT_CARE;
	  i--;
	  j--;
	  /* move back to the one where (state[i]).integer=='1' and switch */
	  while((state[i]).type == 0 && i>=0) {
	    (state[i]).type = 0;
	    (state[i]).bool = DONT_CARE;
	    (state[j]).type = 0;
	    (state[j]).bool = DONT_CARE;
	    i--;
	    j--;
	  }
	  if(i>=0) {
	    (state[i]).type = 0;
	    (state[i]).bool = DONT_CARE;
	    (state[j]).type = 2;
	    (state[j]).integer=getUpperBound(j);
	  }else{
	    printf("We should not arrive here!!!\n");
	    return 0;
	  }
	  i++;
	  j++;
	  (state[i]).type = 2;
	  (state[i]).integer=getUpperBound(i);
	  (state[j]).type = 2;
	  (state[j]).integer=getUpperBound(j);
	}
      }
    }else{
      /* make sure there is no carry */
      (state[i]).type = 2;
      (state[i]).integer=getLowerBound(i);
      (state[j]).type = 2;
      (state[j]).integer=getLowerBound(j);
      for(k=0; k<nicheState+1-(int)(1<<(integerFuncProps->carrySize)); k++) {
	if((state[i]).type == 2 && (state[j]).type == 2) {
	  (state[i]).type = 2;
	  (state[i]).integer=getLowerBound(i);
	  (state[j]).type = 0;
	  (state[j]).integer=DONT_CARE;
	  if(i+2 < integerFuncProps->carrySize) {
	    /* set the next position to both '1' so that there is a carry */
	    i++;
	    j++;
	    (state[i]).type = 2;
	    (state[i]).integer=getLowerBound(i);
	    (state[j]).type = 2;
	    (state[j]).integer=getLowerBound(j);
	  }else if(i+1 < integerFuncProps->carrySize) {
	    i++;
	    j++;
	    (state[i]).type = 2;
	    (state[i]).integer=getLowerBound(i);
	    (state[j]).type = 0;
	    (state[j]).integer=DONT_CARE;
	  }
	}else{
	  /* move back */
	  while((state[i]).integer==DONT_CARE && i>=0) {
	    (state[i]).type = 0;
	    (state[i]).integer=DONT_CARE;
	    (state[j]).type = 0;
	    (state[j]).integer=DONT_CARE;
	    i--;
	    j--;
	  }
	  if(i>=0) {
	    (state[i]).type = 0;
	    (state[i]).integer=DONT_CARE;
	    (state[j]).type = 2;
	    (state[j]).integer=getLowerBound(j);
	  }else{
	    return 0;
	  }
	  if(i+2 < integerFuncProps->carrySize) {
	    /* set the next position to both '0' so that there is no carry */
	    i++;
	    j++;
	    (state[i]).type = 2;
	    (state[i]).integer=getLowerBound(i);
	    (state[j]).type = 2;
	    (state[j]).integer=getLowerBound(j);
	  }else if(i+1 < integerFuncProps->carrySize) {
	    i++;
	    j++;
	    (state[i]).type = 2;
	    (state[i]).integer=getLowerBound(i);
	    (state[j]).type = 0;
	    (state[j]).integer=DONT_CARE;
	  }
	}
      }
    }
    nicheState++;
    return 1;
  }
  return 0;
}

/**
 * Returns the exact estimate for the given condition and action
 */
double getExactEstimate(char *conArray, int class, double *mad)
{
  double av=-1.;
  int i;

  /* this only works for the multiplexer function right now */
  if(integerFuncProps->multiplexerFunction) {
    /* determine all possible outcome positions */
    av = getAveragedMPOutcomes(0, 0, conArray, class);
    if(integerFuncProps->payoffLandscape) {
      *mad = getMADMPOutcomes(0, 0, conArray, class, av);
    }else{
      *mad = 2000. * (av/1000. - av/1000. * av/1000.);
    }
  }else if(integerFuncProps->countOnesFunction) {
    if(integerFuncProps->countOnesType==0) {
      av = getAveragedCO0Outcomes(conArray, class);
      *mad = 2000. * (av/1000. - av/1000. * av/1000.);
    }else{
      av = getAveragedCO1Outcomes(0, conArray, class);
      *mad = getMADCO1Outcomes(0, conArray, class, av);
    }
  }else if(integerFuncProps->classifierFunction) {
    for(i=0; i<integerFuncProps->classifierFunctionSize ; i++) {
      if(conArray[i]==DONT_CARE)
	break;
    }
    av = 500;
    *mad = 500.* (double)(integerFuncProps->classifierFunctionSize-i)/(double)(integerFuncProps->classifierFunctionSize);
    if(i==integerFuncProps->classifierFunctionSize){
      *mad=0.;
      if(class==0) {
	av=0;
      }else{
	av=1000;
      }
    }
  }
  return av;
}

double getAveragedCO0Outcomes(char *conArray, int class)
{
  int i, numOnes, numDCs, numMissing, possible;
  double av;

  numOnes=0;
  numDCs=0;
  for(i=0; i<integerFuncProps->countOnesSize; i++) {
    if(conArray[i]=='1')
      numOnes++;
    else if(conArray[i]!='0')
      numDCs++;
  }
  numMissing = ((integerFuncProps->countOnesSize + 1) / 2)-numOnes;
  if(numMissing<=0) {
    av = 1000.;
  } else {
    possible=0;
    for(i=numMissing; i<= numDCs; i++)
      possible += NChooseK(numDCs, i);
    av = 1000. * (double)((double) possible / ((double)(1<<numDCs)));
  }
  if(class==0)
    av = 1000.-av;

  return av;
}  

double getAveragedCO1Outcomes(int i, char *conArray, int class) 
{
  double result;
  int correct=0;

  if(i<integerFuncProps->countOnesSize) {
    if(conArray[i]=='#') {
      conArray[i]='0';
      result = getAveragedCO1Outcomes(i+1, conArray, class);
      conArray[i]='1';
      result += getAveragedCO1Outcomes(i+1, conArray, class);
      conArray[i] = '#';
      return 0.5*result;
    }else{
      return getAveragedCO1Outcomes(i+1, conArray, class);
    }
  }else{
    return doCountOnesAction(conArray, class, &correct);
  }
}

double getMADCO1Outcomes(int i, char *conArray, int class, double average) 
{
  double result;
  int correct=0;

  if(i<integerFuncProps->countOnesSize) {
    if(conArray[i]=='#') {
      conArray[i]='0';
      result = getMADCO1Outcomes(i+1, conArray, class, average);
      conArray[i]='1';
      result += getMADCO1Outcomes(i+1, conArray, class, average);
      conArray[i] = '#';
      return 0.5*result;
    }else{
      return getMADCO1Outcomes(i+1, conArray, class, average);
    }
  }else{
    return absDouble(average - doCountOnesAction(conArray, class, &correct));
  }
}

double getAveragedMPOutcomes(int i, int pos, char *conArray, int class)
{
  double result;
  int correct=0;
  /* determine ultimate position */
  if(i<integerFuncProps->multiplexerBits) {
    if(conArray[i]=='0') {
      return getAveragedMPOutcomes(i+1, pos, conArray, class);
    }else if(conArray[i]=='1') {
      return getAveragedMPOutcomes(i+1, pos+(1<<(integerFuncProps->multiplexerBits-i-1)), conArray, class);
    }else{
      conArray[i]='0';
      result = getAveragedMPOutcomes(i+1, pos, conArray, class);
      conArray[i]='1';
      result += getAveragedMPOutcomes(i+1, pos+(1<<(integerFuncProps->multiplexerBits-i-1)), conArray, class);
      conArray[i]='#';
      return 0.5*result;
    }
  }
  pos += integerFuncProps->multiplexerBits;
  if(conArray[pos]=='0' || conArray[pos]=='1')
    return doMPAction(conArray, class, &correct);

  conArray[pos]='0';
  result = doMPAction(conArray, class, &correct);
  conArray[pos]='1';
  result += doMPAction(conArray, class, &correct);
  conArray[pos]='#';
  return 0.5*result;
}

double getMADMPOutcomes(int i, int pos, char *conArray, int class, double average)
{
  double result;
  int correct=0;
  if(i<integerFuncProps->multiplexerBits) {
    if(conArray[i]=='0') {
      return getMADMPOutcomes(i+1, pos, conArray, class, average);
    }else if(conArray[i]=='1') {
      return getMADMPOutcomes(i+1, pos+(1<<(integerFuncProps->multiplexerBits-i-1)), conArray, class, average);
    }else{
      conArray[i]='0';
      result = getMADMPOutcomes(i+1, pos, conArray, class, average);
      conArray[i]='1';
      result += getMADMPOutcomes(i+1, pos+(1<<(integerFuncProps->multiplexerBits-i-1)), conArray, class, average);
      conArray[i]='#';
      return 0.5*result;
    }
  }
  pos += integerFuncProps->multiplexerBits;
  if(conArray[pos]=='0' || conArray[pos]=='1')
    return absDouble(average - doMPAction(conArray, class, &correct));

  conArray[pos]='0';
  result = absDouble(average - doMPAction(conArray, class, &correct));
  conArray[pos]='1';
  result += absDouble(average - doMPAction(conArray, class, &correct));
  conArray[pos]='#';
  return 0.5*result;
}

/*
 * initialize the environment -> do nothing in the multiplexer environment 
 */
int initEnv(FILE *fp)
{
  assert((integerFuncProps = (struct integerEnv *) calloc(1,sizeof(struct integerEnv)))!=0);

  integerFuncProps->constantFunction = CONSTANT_FUNCTION;
  integerFuncProps->randomFunction = RANDOM_FUNCTION;
  integerFuncProps->parityFunction = PARITY_FUNCTION;
  integerFuncProps->multiplexerFunction = MULTIPLEXER_FUNCTION;
  integerFuncProps->concatenatedMultiplexer = CONCATENATED_MULTIPLEXER;
  integerFuncProps->biasedMultiplexer = BIASED_MULTIPLEXER;
  integerFuncProps->countOnesFunction = COUNT_ONES_FUNCTION;
  integerFuncProps->carryFunction = CARRY_FUNCTION;
  integerFuncProps->classifierFunction = CLASSIFIER_FUNCTION;

  integerFuncProps->addNoiseToAction = ADD_NOISE_TO_ACTION;
  integerFuncProps->actionNoiseMu = ACTION_NOISE_MU;
  integerFuncProps->actionNoiseSigma = ACTION_NOISE_SIGMA;

  integerFuncProps->samplingBias = SAMPLING_BIAS;

  integerFuncProps->paymentRange = PAYMENT_RANGE;
  integerFuncProps->conditionLength = CONDITION_LENGTH;

  integerFuncProps->paritySize = PARITY_SIZE;

  integerFuncProps->multiplexerBits = MULTIPLEXER_BITS;
  integerFuncProps->payoffLandscape = PAYOFF_LANDSCAPE;

  integerFuncProps->bmpX = BMP_X;
  integerFuncProps->bmpY = BMP_Y;

  integerFuncProps->countOnesSize = COUNT_ONES_SIZE;
  integerFuncProps->countOnesType = COUNT_ONES_TYPE;

  integerFuncProps->carrySize = CARRY_SIZE;

  integerFuncProps->classifierFunctionSize = CLASSIFIER_FUNCTION_SIZE;
  return 1;
}

/**
 * Print the environment constants to file.
 */
void fprintEnv(FILE *outfile)
{
  fprintf(outfile, "# constantFunction %d; randomFunction %d; parityFunction %d; multiplexerFunction %d; concatenatedMP %d; biasedMP %d; countOnesFunction %d; carryFunction %d; classifierFunction %d; addNoiseToAction %f; actionNoiseMu %f; actionNoiseSigma %f; samplingBias %f;\n", integerFuncProps->constantFunction, integerFuncProps->randomFunction, integerFuncProps->parityFunction, integerFuncProps->multiplexerFunction, integerFuncProps->concatenatedMultiplexer, integerFuncProps->biasedMultiplexer, integerFuncProps->countOnesFunction, integerFuncProps->carryFunction, integerFuncProps->classifierFunction, integerFuncProps->addNoiseToAction, integerFuncProps->actionNoiseMu, integerFuncProps->actionNoiseSigma, integerFuncProps->samplingBias); 
  fprintf(outfile, "# paymentRange %d; conditionLength %d; paritySize %d; multiplexerBits %d; payoffLandscape %d; bmpX %d; bmpY %d; countOnesSize %d; countOnesType %d; carrySize %d; classifierFunctionSize %d\n", integerFuncProps->paymentRange, integerFuncProps->conditionLength, integerFuncProps->paritySize, integerFuncProps->multiplexerBits, integerFuncProps->payoffLandscape,integerFuncProps->bmpX, integerFuncProps->bmpY, integerFuncProps->countOnesSize, integerFuncProps->countOnesType, integerFuncProps->carrySize, integerFuncProps->classifierFunctionSize); 
}

/**
 * Resets the all function selections to zero.
 */
void newFunctionChoice()
{
  integerFuncProps->constantFunction = 0;
  integerFuncProps->randomFunction = 0;
  integerFuncProps->multiplexerFunction = 0;
  integerFuncProps->concatenatedMultiplexer = 0;
  integerFuncProps->biasedMultiplexer = 0;
  integerFuncProps->countOnesFunction = 0;
  integerFuncProps->carryFunction = 0;
  integerFuncProps->classifierFunction = 0;
}

/**
 * Tries to set parameter 'type' to value 'value. If 'type' does not exist, 0 is returned.
 */
int setEnvParam(char *type, double value)
{
  if(strcmp(type,"constantFunction")==0){
    if((int)value){
      integerFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    integerFuncProps->constantFunction = (int)value;
  }else if(strcmp(type,"randomFunction")==0){
    if((int)value){
      integerFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    integerFuncProps->randomFunction = (int)value;
  }else if(strcmp(type,"parityFunction")==0){
    if((int)value){
      integerFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    integerFuncProps->parityFunction = (int)value;
  }else if(strcmp(type,"multiplexerFunction")==0){
    if((int)value){
      integerFuncProps->paymentRange = PAYMENT_RANGE;
      if(integerFuncProps->payoffLandscape) /* redetermine payment range if set */
	integerFuncProps->paymentRange = (1<<((integerFuncProps->multiplexerBits)+1))*100+200;
      newFunctionChoice();
    }
    integerFuncProps->multiplexerFunction = (int)value;
  }else if(strcmp(type,"concatenatedMultiplexer")==0) {
    if((int)value) {
      integerFuncProps->paymentRange = PAYMENT_RANGE;
      if(integerFuncProps->payoffLandscape) /* redetermine payment range if set */
	/* size of condition divided by size of one multiplexer */
	integerFuncProps->paymentRange = 1000*(integerFuncProps->conditionLength) / (integerFuncProps->multiplexerBits + (1<<integerFuncProps->multiplexerBits)); 
      newFunctionChoice();
    }
    integerFuncProps->concatenatedMultiplexer = (int)value;
  }else if(strcmp(type,"biasedMultiplexer")==0){
    if((int)value) {
      integerFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    integerFuncProps->biasedMultiplexer = (int)value;
  }else if(strcmp(type,"countOnesFunction")==0){
    if((int)value){
      integerFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    integerFuncProps->countOnesFunction = (int)value;
  }else if(strcmp(type,"carryFunction")==0){
    if((int)value){
      integerFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    integerFuncProps->carryFunction = (int)value;
  }else if(strcmp(type,"classifierFunction")==0){
    if((int)value){
      integerFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    integerFuncProps->classifierFunction = (int)value;
  }else if(strcmp(type,"addNoiseToAction")==0){
    integerFuncProps->addNoiseToAction = value;
  }else if(strcmp(type,"actionNoiseMu")==0){
    integerFuncProps->actionNoiseMu = value;
  }else if(strcmp(type,"actionNoiseSigma")==0){
    integerFuncProps->actionNoiseSigma = value;
  }else if(strcmp(type,"samplingBias")==0){
    integerFuncProps->samplingBias = value;
  }else if(strcmp(type,"paymentRange")==0){
    printf("Payment Range is set internally!\n");
    /*integerFuncProps->paymentRange = (int)value;*/
  }else if(strcmp(type,"conditionLength")==0){
    integerFuncProps->conditionLength = (int)value;
  }else if(strcmp(type,"paritySize")==0){
    integerFuncProps->paritySize = (int)value;
  }else if(strcmp(type,"multiplexerBits")==0){
    integerFuncProps->multiplexerBits = (int)value;
    if(integerFuncProps->payoffLandscape) { /* redetermine payment range if set */
      if(integerFuncProps->multiplexerFunction)
	integerFuncProps->paymentRange = (1<<((integerFuncProps->multiplexerBits)+1))*100+200;
      else if(integerFuncProps->concatenatedMultiplexer)
	integerFuncProps->paymentRange = 1000*(integerFuncProps->conditionLength)/
	  (integerFuncProps->multiplexerBits + (1<<integerFuncProps->multiplexerBits)); 
    }
  }else if(strcmp(type,"payoffLandscape")==0){
    integerFuncProps->payoffLandscape = (int)value;
    if(integerFuncProps->payoffLandscape) { /* redetermine payment range if set */
      if(integerFuncProps->multiplexerFunction)
	integerFuncProps->paymentRange = (1<<((integerFuncProps->multiplexerBits)+1))*100+200;
      else if(integerFuncProps->concatenatedMultiplexer)
	integerFuncProps->paymentRange = 1000*(integerFuncProps->conditionLength)/
	  (integerFuncProps->multiplexerBits + (1<<integerFuncProps->multiplexerBits)); 
    }
  }else if(strcmp(type,"bmpX")==0){
    integerFuncProps->bmpX = (int)value;
  }else if(strcmp(type,"bmpY")==0){
    integerFuncProps->bmpY = (int)value;
  }else if(strcmp(type,"countOnesSize")==0){
    integerFuncProps->countOnesSize = (int)value;
  }else if(strcmp(type,"countOnesType")==0){
    integerFuncProps->countOnesType = (int)value;
  }else if(strcmp(type,"carrySize")==0){
    integerFuncProps->carrySize = (int)value;
  }else if(strcmp(type,"classifierFunctionSize")==0){
    integerFuncProps->classifierFunctionSize = (int)value;
  }else{
    printf("%s not supported in current environment\n",type);
    return 0;
  }
  return 1;
}

/**
 * Default to free any alocated memory. 
 */
void freeEnv()
{
  free(integerFuncProps);
  return;
}
