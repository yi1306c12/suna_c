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
#include "boolFktEnv.h"
#include "xcsMacros.h"

/* the current properties of this boolean function */
struct booleanEnv *boolFuncProps = 0;
double getMADMPOutcomes(int i, int pos, char *conArray, int class, double average);
double getAveragedMPOutcomes(int i, int pos, char *conArray, int class);
double getAveragedCO0Outcomes(char *conArray, int class);
double getAveragedCO1Outcomes(int i, char *conArray, int class);
double getMADCO1Outcomes(int i, char *conArray, int class, double average);
int bfeNicheState=0;
int bfeTestState=0;

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
  return boolFuncProps->conditionLength;
}

/**
 * Returns the type (int, boolean, real) for the specified type.
 */
int getAttributeType(int att)
{
  return 0; /* this is the normal Boolean type */
}

/**
 * Returns the upper bound of an attribute.
 */
double getUpperBound(int att)
{
  return 1.; /* in the Boolean case this is always one */
}

/**
 * Returns the lower bound of an attribute.
 */
double getLowerBound(int att)
{
  return 0; /* in the boolean case this is always zero */
}

/**
 * Returns the payment range of this problem (i.e. the maximum payoff).
 */
int getPaymentRange()
{
  return boolFuncProps->paymentRange;
}

/**
 * Returns the number of possible actions.
 */
int getNumberOfActions()
{
  if(boolFuncProps->concatenatedMultiplexer==1)
    return (int)(1<<(int)((boolFuncProps->conditionLength)/(boolFuncProps->multiplexerBits + (1<<boolFuncProps->multiplexerBits))));
  return 2;
}

/**
 * Resets the state to a new problem instance.
 */ 
void resetState(struct attributes *state)
{
  int i, redo=1;
  int action=0, correct=0;
  bfeNicheState=0;
  /* generate a random state */
  while(redo) {
    for(i=0; i < boolFuncProps->conditionLength; i++) {
      state[i].bool = (char)(((double) rand() / (RAND_MAX+1.0))*2)+'0';
    }
    if(boolFuncProps->samplingBias == 0.5) {
      redo = 0;
    }else{
      doAction(state, action, &correct);
      /* accept a string with result '0' with probability samplingBias */
      redo=0;
      if(correct==1) {
	if(urand() < boolFuncProps->samplingBias)
	  redo = 1;
      }else{
	if(urand() > boolFuncProps->samplingBias)
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
  
  assert((state = calloc(getConditionLength(), sizeof(char)))!=NULL);
  for(i=0; i<getConditionLength(); i++)
    state[i] = ss[i].bool;

  /* return the perceived reward */
  if(boolFuncProps->constantFunction) {
    *correct = 1;
    value = (double)boolFuncProps->paymentRange;
  }else if(boolFuncProps->randomFunction)
    value = doRandomAction(state,act,correct);
  else if(boolFuncProps->parityFunction)
    value = doParityAction(state,act,correct);
  else if(boolFuncProps->multiplexerFunction)
    value = doMPAction(state,act,correct);
  else if(boolFuncProps->concatenatedMultiplexer)
    value = doConcatenatedMultiplexerAction(state, act, correct);
  else if(boolFuncProps->biasedMultiplexer)
    value = doBiasedMultiplexerAction(state,act,correct);
  else if(boolFuncProps->countOnesFunction)
    value = doCountOnesAction(state, act, correct);
  else if(boolFuncProps->combinedParityCOorMPFunction)
    value = doCombinedParityCOorMPAction(state, act, correct);
  else if(boolFuncProps->carryFunction)
    value = doCarryAction(state, act, correct);
  else if(boolFuncProps->classifierFunction)
    value = doClassifierAction(state, act, correct);
  else{
    value = 0;
    printf("Some function should have been chosen in env.h or set in the input file!\n");
  }

  if(boolFuncProps->addNoiseToAction>0) {
    if(boolFuncProps->addNoiseToAction<4) { /* Add Gaussian noise to outcome value */
      if(  boolFuncProps->addNoiseToAction==1 || (boolFuncProps->addNoiseToAction==2 && act==0) || (boolFuncProps->addNoiseToAction==3 && act==1))
	value += (boolFuncProps->actionNoiseMu) + nrand() * (boolFuncProps->actionNoiseSigma);
    }else{ /* Alternate outcome with a certain probabilitiy possibly action dependent */
      if(  boolFuncProps->addNoiseToAction<=5 || (boolFuncProps->addNoiseToAction<=6 && act==0) || (boolFuncProps->addNoiseToAction>6 && boolFuncProps->addNoiseToAction<=7 && act==1)) {
	if(urand() < boolFuncProps->addNoiseToAction-(double)(int)(boolFuncProps->addNoiseToAction)) {
	  if(value == boolFuncProps->paymentRange)
	    value = 0;
	  else
	    value = boolFuncProps->paymentRange;
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
  return (double)((int)(((double) rand() / (RAND_MAX+1.0)) *2) * (boolFuncProps->paymentRange));
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

double doCombinedParityCOorMPAction(char *state, int act, int *correct)
{
  char *helpState;
  int i, length, cor;
  double ret;

  length = (boolFuncProps->conditionLength) / (boolFuncProps->paritySize);
  assert((helpState = (char*)calloc(length, sizeof(char)))!=NULL);

  cor=0;
  for(i=0; i<length; i++) {
    doParityAction(&(state[i*boolFuncProps->paritySize]), 1, &cor);
    helpState[i] = (char)((int)'0'+cor);
  }
  if(boolFuncProps->combinedType==0) {
    ret = doCountOnesAction(helpState, act, correct);
  }else{
    ret = doMPAction(helpState, act, correct);
  }
  free(helpState);
  return ret;
}

double doCarryAction(char *state, int act, int *correct)
{
  int i,j;
  int carry=0;

  j=boolFuncProps->carrySize;
  for(i=0; i<boolFuncProps->carrySize; i++,j++) {
    if(state[i]=='0' && state[j]=='0') {
      break;
    }else if(state[i]=='1' && state[j]=='1') {
      carry=1;
      break;
    }
  }
  
  if(act==carry) {
    *correct=1;
    return boolFuncProps->paymentRange;
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
  for(i=0, j=0; i<boolFuncProps->countOnesSize; i++){
    if(state[i]=='1')/* count the number of ones */
      j++;
  }
  switch(boolFuncProps->countOnesType){
  case 0: /* the number of ones determines the outcome */
    if(j*2 >= boolFuncProps->countOnesSize){ /* this is the action one */
      if(act==1){
	*correct = 1;
	return boolFuncProps->paymentRange;
      }else{
	*correct = 0;
	return 0;
      }
    }else { /* If equal or less, the action should be zero! */
      if(act==0){
	*correct = 1;
	return boolFuncProps->paymentRange;
      }else{
	*correct = 0;
	return 0;
      }
    }
    break;
  case 1: /* The number of ones determines the payoff level returned */
    if(j*2 > boolFuncProps->countOnesSize){ /* this is the action one */
      if(act==1) {
	*correct = 1;
	return (boolFuncProps->paymentRange) * j / (boolFuncProps->countOnesSize);
      }else{
	*correct = 0;
	return (boolFuncProps->paymentRange) * ((boolFuncProps->countOnesSize)-j) / (boolFuncProps->countOnesSize);
      }
    }else if(j*2 == boolFuncProps->countOnesSize) { /* 50/50 here - with this it is not possible to distinguish between action - so either action is correct!*/
      *correct = 1;
      return (boolFuncProps->paymentRange)/2;
    }else{
      if(act==0){
	*correct = 1;
	return (boolFuncProps->paymentRange)*((boolFuncProps->countOnesSize)-j) / (boolFuncProps->countOnesSize);
      }else{
	*correct = 0;
	return (boolFuncProps->paymentRange) * j / (boolFuncProps->countOnesSize);
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
  for(i=0; i<boolFuncProps->paritySize; i++){
    if(state[i]=='1')
      sum++;
  }
  if(sum%2==0)
    if(act==0){
      *correct =1;
      return boolFuncProps->paymentRange;
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
      return boolFuncProps->paymentRange;
    }
}

/**
 * Multiplexer Function
 */
double doMPAction(char *state, int act, int *correct)
{
  int place=boolFuncProps->multiplexerBits;
  int i;
  double reward;
 
  /* get the place of the by the first index bits referenced spot */
  for(i=0,place=boolFuncProps->multiplexerBits; i<boolFuncProps->multiplexerBits; i++) {
    if(state[i]=='1')
      place += (int)(1<<((boolFuncProps->multiplexerBits)-1-i));
  }
  /* determine the corresponding reward and set 'correct' */
  if((act==1 && state[place]=='1') || (act==0 && state[place]=='0')) {
    /* the correct classification was chosen */
    *correct=1;
    if(boolFuncProps->payoffLandscape)
      reward= 300.+(double)(((place-(boolFuncProps->multiplexerBits))*200)+100*(int)(state[place]-'0'));
    else
      reward = boolFuncProps->paymentRange;
  }else{
    /* the incorrect classification was chosen */
    *correct=0;
    if(boolFuncProps->payoffLandscape)
      reward= 0.+(double)(((place - (boolFuncProps->multiplexerBits))*200)+100*(int)(state[place]-'0'));
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

  for(i=0,place=0; i<boolFuncProps->bmpX; i++){
    if(state[i]=='1')
      place += (int)(1<<(boolFuncProps->bmpX-i-1));
  }
  if(place >= ((int)(1<<(boolFuncProps->bmpX)))/2)
    zeroone=1;
  increment=((int)(1<<(boolFuncProps->bmpY))) - 1 + boolFuncProps->bmpY;
  for(placelocal=0, i=0; i< boolFuncProps->bmpY; i++){
    if(state[boolFuncProps->bmpX + place * increment + i]=='1')
      placelocal += (int)(1<<(boolFuncProps->bmpY-i-1));
  }
  if(zeroone==0){
    if(placelocal==0){
      action='0';
    }else{
      placelocal--;
      action=state[boolFuncProps->bmpX + place*increment + boolFuncProps->bmpY + placelocal];
    }
  }else{
    if(placelocal == ((int)(1<<boolFuncProps->bmpY))-1){
      action='1';
    }else{
      action=state[boolFuncProps->bmpX + place*increment + boolFuncProps->bmpY + placelocal];
    }
  }
  /* printf("%s -> %c\n",state,action);*/
 if((action=='1'&& act==1) || (action=='0' && act==0)){
    *correct=1;
    return boolFuncProps->paymentRange;
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
  int place=boolFuncProps->multiplexerBits;
  int nrMPs = (int)((boolFuncProps->conditionLength)/(boolFuncProps->multiplexerBits + (1<<boolFuncProps->multiplexerBits)));
  int length = boolFuncProps->multiplexerBits + (1<<boolFuncProps->multiplexerBits);
  int i,j, nOne;
  double reward;
 
  *correct=1;
  reward=0;
  nOne=0;
  for(i=0; i<nrMPs; i++) {
    /* get the place of the spot referenced by the first index bits */
    for(j=0, place=length*i+boolFuncProps->multiplexerBits; j < boolFuncProps->multiplexerBits; j++) {
      if(state[length*i+j]=='1')
	place += (int)(1<<((boolFuncProps->multiplexerBits)-1-j));
    }
    /* determine the corresponding reward and set 'correct' */
    if(boolFuncProps->concatenatedMultiplexer==1) {
      if((act==1 && state[place]=='1') || (act==0 && state[place]=='0')) {
	/* the right action was chosen */
	if(boolFuncProps->payoffLandscape)
	  reward += 1000;
      }else{
	/* the wrong action was chosen */
	*correct=0;
	if(!boolFuncProps->payoffLandscape)
	  break; /* if one is wrong and no payoff Landscape, all is wrong */
      }
    }else{
      if(state[place]=='1') {
	nOne++;
      }
    }
  }
  if(boolFuncProps->concatenatedMultiplexer==2) {
    if((nOne>1 && act==1) || (nOne<2 && act==0)) {/*type with only one action*/
      /*printf("Is correct %s - %s: %d\n",state, act, nOne);*/
      *correct = 1;
      reward = 1000;
    }else{
      *correct = 0;
      reward = 0;
    }
    if(boolFuncProps->payoffLandscape) {
      if(act==1)
	reward = 1000 * nOne;
      else
	reward = 1000 * (nrMPs-nOne);
    }
  }else{
    if(*correct && !boolFuncProps->payoffLandscape)
      reward = 1000;
    /* printf("%s with act %s was assigned %d with reward %f\n",state, act, *correct, reward);*/
  }
  return reward;
}



/**
 * Returns a state of the next to-be tested niche. 
 * Type=0 returns the exact schema of the niche
 * Type=1 returns a sample from that niche.
 */
int getNextNiche(struct attributes *state, int type)
{
  int num, i, j, k, l, move, res, parity;
  struct attributes *helpState;
  char *parityType;
  
  if(boolFuncProps->multiplexerFunction || 
     boolFuncProps->countOnesFunction ||
     boolFuncProps->carryFunction ||
     boolFuncProps->combinedParityCOorMPFunction) {
    if(boolFuncProps->multiplexerFunction) {
      /* Multiplexer Function */
      num = (int)(1<<(boolFuncProps->multiplexerBits + 1));
      if(num <= bfeNicheState) /* no more niches */
	return 0;
      j=bfeNicheState;
      for(i=0; i<boolFuncProps->multiplexerBits; i++) {
	if( j >= (int)(1<<(boolFuncProps->multiplexerBits-i))) {
	  (state[i]).bool='1';
	  j -= (int)(1<<(boolFuncProps->multiplexerBits-i));
	}else{
	  (state[i]).bool='0';
	}
      }
      for(j=0; j<(1<<(boolFuncProps->multiplexerBits)); j++)
	(state[boolFuncProps->multiplexerBits+j]).bool = DONT_CARE;
      j = bfeNicheState/2;
      (state[boolFuncProps->multiplexerBits+j]).bool = '0'+(bfeNicheState%2);
    }else if(boolFuncProps->countOnesFunction) {
      if(boolFuncProps->countOnesType==1) {
	/* Count ones problem type 1 */
	num = (int)(1<<boolFuncProps->countOnesSize);
	if(num <= bfeNicheState)
	  return 0;
	for(i=0; i<boolFuncProps->conditionLength; i++) 
	  (state[i]).bool=DONT_CARE;
	j=bfeNicheState;
	for(i=0; i<boolFuncProps->countOnesSize; i++) {
	  if( j >= (int)(1<<(boolFuncProps->countOnesSize-i-1))) {
	    (state[i]).bool='1';
	    j -= (int)(1<<(boolFuncProps->multiplexerBits-i-1));
	  }else{
	    (state[i]).bool='0';
	  }
	}
      }else{
	/* Count ones problem type 0 */
	num = 2 * NChooseK(boolFuncProps->countOnesSize, (boolFuncProps->countOnesSize-1)/2 );
	if(num <= bfeNicheState)
	  return 0;
	
	/* MORE NICHES TO COME */
	for(i=0; i<boolFuncProps->conditionLength; i++) 
	  (state[i]).bool=DONT_CARE;
	/* zero or one niche */
	if(bfeNicheState < num/2) {
	  (state[0]).bool='0';
	  j=bfeNicheState;
	}else{
	  (state[0]).bool='1';
	  j=bfeNicheState - num/2;
	}
	for(i=1; i<(boolFuncProps->countOnesSize+1)/2; i++)
	  (state[i]).bool=(state[i-1]).bool;
	/* iteratively generate actual test niche */
	for(k=0; k<j; k++) {
	  if((state[boolFuncProps->countOnesSize-1]).bool == DONT_CARE) {
	    /* just move last entry */
	    for( i = 1; i<boolFuncProps->countOnesSize ; i++) {
	      if((state[boolFuncProps->countOnesSize-1-i]).bool != DONT_CARE){
		(state[boolFuncProps->countOnesSize-i]).bool = (state[boolFuncProps->countOnesSize-1-i]).bool;
		(state[boolFuncProps->countOnesSize-1-i]).bool = DONT_CARE;
		break; 
	      }
	    }
	  }else{
	    /* move chunk to next state */
	    (state[boolFuncProps->countOnesSize-1]).bool = DONT_CARE;
	    /* detect chunk to be moved */
	    for( i = 1; i<boolFuncProps->countOnesSize; i++) {
	      if( (state[boolFuncProps->countOnesSize-1-i]).bool == DONT_CARE) {
		move = i;
		if(move == (boolFuncProps->countOnesSize+1)/2 ){
		  printf("Seems to be done!?\n");
		  return 0;
		}
		break;
	      }else{
		(state[boolFuncProps->countOnesSize-1-i]).bool = DONT_CARE;
	      }
	    }
	    /* move specifies the number of guys that will be moved back */
	    /* look for place where to move it */
	    for(i++; i<boolFuncProps->countOnesSize; i++) {
	      if((state[boolFuncProps->countOnesSize-1-i]).bool!=DONT_CARE) {
		/* found it */
		(state[boolFuncProps->countOnesSize-i]).bool=(state[boolFuncProps->countOnesSize-1-i]).bool;
		(state[boolFuncProps->countOnesSize-1-i]).bool=DONT_CARE;
		for(l=0; l<move; l++) {
		  (state[boolFuncProps->countOnesSize-i+1+l]).bool = (state[boolFuncProps->countOnesSize-i+l]).bool;
		}
		break;
	      }
	    }
	  }
	  /* next iteration */
	}
      }
    }else if(boolFuncProps->carryFunction) {
      /* MORE NICHES TO COME */
      for(i=0; i<boolFuncProps->conditionLength; i++) 
	(state[i]).bool=DONT_CARE;
      i=0;
      j=boolFuncProps->carrySize;
      
      if(bfeNicheState+1 < (int)(1<<(boolFuncProps->carrySize))) {
	(state[i]).bool='1';
	(state[j]).bool='1';
	for(k=0; k<bfeNicheState; k++) {
	  (state[i]).bool='1';
	  (state[j]).bool=DONT_CARE;
	  if(i+1 < boolFuncProps->carrySize) {
	    /* set the next position to both '1' so that there is a carry */
	    i++;
	    j++;
	    (state[i]).bool='1';
	    (state[j]).bool='1';
	  }else{
	    (state[i]).bool=DONT_CARE;
	    (state[j]).bool=DONT_CARE;
	    i--;
	    j--;
	    /* move back to the one where (state[i]).bool=='1' and switch */
	    while((state[i]).bool==DONT_CARE && i>=0) {
	      (state[i]).bool=DONT_CARE;
	      (state[j]).bool=DONT_CARE;
	      i--;
	      j--;
	    }
	    if(i>=0) {
	      (state[i]).bool=DONT_CARE;
	      (state[j]).bool='1';
	    }else{
	      printf("We should not arrive here!!!\n");
	      return 0;
	    }
	    i++;
	    j++;
	    (state[i]).bool='1';
	    (state[j]).bool='1';
	  }
	}
      }else{
	/* make sure there is no carry */
	(state[i]).bool='0';
	(state[j]).bool='0';
	for(k=0; k<bfeNicheState+1-(int)(1<<(boolFuncProps->carrySize)); k++) {
	  if((state[i]).bool=='0' && (state[j]).bool=='0') {
	    (state[i]).bool='0';
	    (state[j]).bool=DONT_CARE;
	    if(i+2 < boolFuncProps->carrySize) {
	      /* set the next position to both '1' so that there is a carry */
	      i++;
	      j++;
	      (state[i]).bool='0';
	      (state[j]).bool='0';
	    }else if(i+1 < boolFuncProps->carrySize) {
	      i++;
	      j++;
	      (state[i]).bool='0';
	      (state[j]).bool=DONT_CARE;
	    }
	  }else{
	    /* move back */
	    while((state[i]).bool==DONT_CARE && i>=0) {
	      (state[i]).bool=DONT_CARE;
	      (state[j]).bool=DONT_CARE;
	      i--;
	      j--;
	    }
	    if(i>=0) {
	      (state[i]).bool=DONT_CARE;
	      (state[j]).bool='0';
	    }else{
	      return 0;
	    }
	    if(i+2 < boolFuncProps->carrySize) {
	      /* set the next position to both '0' so that there is no carry */
	      i++;
	      j++;
	      (state[i]).bool='0';
	      (state[j]).bool='0';
	    }else if(i+1 < boolFuncProps->carrySize) {
	      i++;
	      j++;
	      (state[i]).bool='0';
	      (state[j]).bool=DONT_CARE;
	    }
	  }
	}
      }
    }else if(boolFuncProps->combinedParityCOorMPFunction) {
      /* combined Parity + CO/MP function */
      boolFuncProps->combinedParityCOorMPFunction = 0;
      if(boolFuncProps->combinedType==0) {
	boolFuncProps->countOnesFunction=1;
      }else{
	boolFuncProps->multiplexerFunction=1;
      }
      assert((helpState = (struct attributes *)calloc(boolFuncProps->conditionLength, sizeof(struct attributes)))!=NULL);
      for(i=0; i<boolFuncProps->conditionLength; i++) {
	(helpState[i]).bool = DONT_CARE;
      }
      res = getNextNiche(helpState, type);
      bfeNicheState--;
      boolFuncProps->countOnesFunction=0;
      boolFuncProps->multiplexerFunction=0;
      boolFuncProps->combinedParityCOorMPFunction = 1;
      if(res==0) { /* No more niches to come */
	free(helpState);
	return 0;
      }
      assert((parityType = (char *)calloc(boolFuncProps->paritySize, sizeof(char)))!=NULL);
      for(i=0; i<boolFuncProps->conditionLength; i+=boolFuncProps->paritySize) {
	do{
	  parity=0;
	  for(j=0; j<boolFuncProps->paritySize; j++) {
	    parityType[j] = (char)((int)('0') + (int)(urand()*2));
	    if(parityType[j]=='1')
	      parity++;
	  }
	}while(parity%2 != (int)(helpState[(int)(i/(boolFuncProps->paritySize))].bool)-(int)('0'));
	for(j=0; j<boolFuncProps->paritySize; j++) {
	  state[i+j].bool = parityType[j];
	}
      }
      free(parityType);
      free(helpState);
    }
    /* Additionally fill dont'case symbols if type==1 */
    if(type==1) {
      for(i=0; i<boolFuncProps->conditionLength; i++) {
	if((state[i]).bool == DONT_CARE) {
	  if(urand() <0.5) {
	    (state[i]).bool = '0';
	  }else{
	    (state[i]).bool = '1';
	  }
	}
      }
    }
    bfeNicheState++;
    return 1;
  }
  return 0;
}

/**
 * Returns the exact estimate for the given condition and action
 */
double getExactEstimate(struct condAttribute *cA, int class, double *mad)
{
  double av=-1.;
  int i;
  char *conArray;
  
  assert((conArray = (char *)calloc(boolFuncProps->conditionLength+1, sizeof(char)))!=NULL);
  for(i=0; i<boolFuncProps->conditionLength; i++) {
    conArray[i] = cA[i].bool;
  }
  
  /* this only works for the multiplexer function right now */
  if(boolFuncProps->multiplexerFunction) {
    /* determine all possible outcome positions */
    av = getAveragedMPOutcomes(0, 0, conArray, class);
    if(boolFuncProps->payoffLandscape) {
      *mad = getMADMPOutcomes(0, 0, conArray, class, av);
    }else{
      *mad = 2000. * (av/1000. - av/1000. * av/1000.);
    }
  }else if(boolFuncProps->countOnesFunction) {
    if(boolFuncProps->countOnesType==0) {
      av = getAveragedCO0Outcomes(conArray, class);
      *mad = 2000. * (av/1000. - av/1000. * av/1000.);
    }else{
      av = getAveragedCO1Outcomes(0, conArray, class);
      *mad = getMADCO1Outcomes(0, conArray, class, av);
    }
  }else if(boolFuncProps->classifierFunction) {
    for(i=0; i<boolFuncProps->classifierFunctionSize ; i++) {
      if(conArray[i]==DONT_CARE)
	break;
    }
    av = 500;
    *mad = 500.* (double)(boolFuncProps->classifierFunctionSize-i)/(double)(boolFuncProps->classifierFunctionSize);
    if(i==boolFuncProps->classifierFunctionSize){
      *mad=0.;
      if(class==0) {
	av=0;
      }else{
	av=1000;
      }
    }
  }

  free(conArray);
  return av;
}

double getAveragedCO0Outcomes(char *conArray, int class)
{
  int i, numOnes, numDCs, numMissing, possible;
  double av;

  numOnes=0;
  numDCs=0;
  for(i=0; i<boolFuncProps->countOnesSize; i++) {
    if(conArray[i]=='1')
      numOnes++;
    else if(conArray[i]!='0')
      numDCs++;
  }
  numMissing = ((boolFuncProps->countOnesSize + 1) / 2)-numOnes;
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

  if(i<boolFuncProps->countOnesSize) {
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

  if(i<boolFuncProps->countOnesSize) {
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
  if(i<boolFuncProps->multiplexerBits) {
    if(conArray[i]=='0') {
      return getAveragedMPOutcomes(i+1, pos, conArray, class);
    }else if(conArray[i]=='1') {
      return getAveragedMPOutcomes(i+1, pos+(1<<(boolFuncProps->multiplexerBits-i-1)), conArray, class);
    }else{
      conArray[i]='0';
      result = getAveragedMPOutcomes(i+1, pos, conArray, class);
      conArray[i]='1';
      result += getAveragedMPOutcomes(i+1, pos+(1<<(boolFuncProps->multiplexerBits-i-1)), conArray, class);
      conArray[i]='#';
      return 0.5*result;
    }
  }
  pos += boolFuncProps->multiplexerBits;
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
  if(i<boolFuncProps->multiplexerBits) {
    if(conArray[i]=='0') {
      return getMADMPOutcomes(i+1, pos, conArray, class, average);
    }else if(conArray[i]=='1') {
      return getMADMPOutcomes(i+1, pos+(1<<(boolFuncProps->multiplexerBits-i-1)), conArray, class, average);
    }else{
      conArray[i]='0';
      result = getMADMPOutcomes(i+1, pos, conArray, class, average);
      conArray[i]='1';
      result += getMADMPOutcomes(i+1, pos+(1<<(boolFuncProps->multiplexerBits-i-1)), conArray, class, average);
      conArray[i]='#';
      return 0.5*result;
    }
  }
  pos += boolFuncProps->multiplexerBits;
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
  assert((boolFuncProps = (struct booleanEnv *) calloc(1,sizeof(struct booleanEnv)))!=0);

  boolFuncProps->constantFunction = CONSTANT_FUNCTION;
  boolFuncProps->randomFunction = RANDOM_FUNCTION;
  boolFuncProps->parityFunction = PARITY_FUNCTION;
  boolFuncProps->multiplexerFunction = MULTIPLEXER_FUNCTION;
  boolFuncProps->concatenatedMultiplexer = CONCATENATED_MULTIPLEXER;
  boolFuncProps->biasedMultiplexer = BIASED_MULTIPLEXER;
  boolFuncProps->countOnesFunction = COUNT_ONES_FUNCTION;
  boolFuncProps->carryFunction = CARRY_FUNCTION;
  boolFuncProps->combinedParityCOorMPFunction = COMBINED_PARITY_MP_OR_CO;
  boolFuncProps->classifierFunction = CLASSIFIER_FUNCTION;

  boolFuncProps->addNoiseToAction = ADD_NOISE_TO_ACTION;
  boolFuncProps->actionNoiseMu = ACTION_NOISE_MU;
  boolFuncProps->actionNoiseSigma = ACTION_NOISE_SIGMA;

  boolFuncProps->samplingBias = SAMPLING_BIAS;

  boolFuncProps->paymentRange = PAYMENT_RANGE;
  boolFuncProps->conditionLength = CONDITION_LENGTH;

  boolFuncProps->paritySize = PARITY_SIZE;

  boolFuncProps->multiplexerBits = MULTIPLEXER_BITS;
  boolFuncProps->payoffLandscape = PAYOFF_LANDSCAPE;

  boolFuncProps->bmpX = BMP_X;
  boolFuncProps->bmpY = BMP_Y;

  boolFuncProps->countOnesSize = COUNT_ONES_SIZE;
  boolFuncProps->countOnesType = COUNT_ONES_TYPE;

  boolFuncProps->combinedType = COMBINED_TYPE;

  boolFuncProps->carrySize = CARRY_SIZE;

  boolFuncProps->classifierFunctionSize = CLASSIFIER_FUNCTION_SIZE;
  return 1;
}

/**
 * Print the environment constants to file.
 */
void fprintEnv(FILE *outfile)
{
  fprintf(outfile, "# constantFunction %d; randomFunction %d; parityFunction %d; multiplexerFunction %d; concatenatedMP %d; biasedMP %d; countOnesFunction %d; combinedParityCOorMPFunction %d; carryFunction %d; classifierFunction %d; addNoiseToAction %f; actionNoiseMu %f; actionNoiseSigma %f; samplingBias %f;\n", boolFuncProps->constantFunction, boolFuncProps->randomFunction, boolFuncProps->parityFunction, boolFuncProps->multiplexerFunction, boolFuncProps->concatenatedMultiplexer, boolFuncProps->biasedMultiplexer, boolFuncProps->countOnesFunction, boolFuncProps->combinedParityCOorMPFunction, boolFuncProps->carryFunction, boolFuncProps->classifierFunction, boolFuncProps->addNoiseToAction, boolFuncProps->actionNoiseMu, boolFuncProps->actionNoiseSigma, boolFuncProps->samplingBias); 
  fprintf(outfile, "# paymentRange %d; conditionLength %d; paritySize %d; multiplexerBits %d; payoffLandscape %d; bmpX %d; bmpY %d; countOnesSize %d; countOnesType %d; combinedType %d; carrySize %d; classifierFunctionSize %d\n", boolFuncProps->paymentRange, boolFuncProps->conditionLength, boolFuncProps->paritySize, boolFuncProps->multiplexerBits, boolFuncProps->payoffLandscape,boolFuncProps->bmpX, boolFuncProps->bmpY, boolFuncProps->countOnesSize, boolFuncProps->countOnesType, boolFuncProps->combinedType, boolFuncProps->carrySize, boolFuncProps->classifierFunctionSize); 
}

/**
 * Resets the all function selections to zero.
 */
void newFunctionChoice()
{
  boolFuncProps->constantFunction = 0;
  boolFuncProps->randomFunction = 0;
  boolFuncProps->multiplexerFunction = 0;
  boolFuncProps->concatenatedMultiplexer = 0;
  boolFuncProps->biasedMultiplexer = 0;
  boolFuncProps->countOnesFunction = 0;
  boolFuncProps->carryFunction = 0;
  boolFuncProps->combinedParityCOorMPFunction = 0;
  boolFuncProps->classifierFunction = 0;
}

/**
 * Tries to set parameter 'type' to value 'value. If 'type' does not exist, 0 is returned.
 */
int setEnvParam(char *type, double value)
{
  if(strcmp(type,"constantFunction")==0){
    if((int)value){
      boolFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    boolFuncProps->constantFunction = (int)value;
  }else if(strcmp(type,"randomFunction")==0){
    if((int)value){
      boolFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    boolFuncProps->randomFunction = (int)value;
  }else if(strcmp(type,"parityFunction")==0){
    if((int)value){
      boolFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    boolFuncProps->parityFunction = (int)value;
  }else if(strcmp(type,"multiplexerFunction")==0){
    if((int)value){
      boolFuncProps->paymentRange = PAYMENT_RANGE;
      if(boolFuncProps->payoffLandscape) /* redetermine payment range if set */
	boolFuncProps->paymentRange = (1<<((boolFuncProps->multiplexerBits)+1))*100+200;
      newFunctionChoice();
    }
    boolFuncProps->multiplexerFunction = (int)value;
  }else if(strcmp(type,"concatenatedMultiplexer")==0) {
    if((int)value) {
      boolFuncProps->paymentRange = PAYMENT_RANGE;
      if(boolFuncProps->payoffLandscape) /* redetermine payment range if set */
	/* size of condition divided by size of one multiplexer */
	boolFuncProps->paymentRange = 1000*(boolFuncProps->conditionLength) / (boolFuncProps->multiplexerBits + (1<<boolFuncProps->multiplexerBits)); 
      newFunctionChoice();
    }
    boolFuncProps->concatenatedMultiplexer = (int)value;
  }else if(strcmp(type,"biasedMultiplexer")==0){
    if((int)value) {
      boolFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    boolFuncProps->biasedMultiplexer = (int)value;
  }else if(strcmp(type,"countOnesFunction")==0){
    if((int)value){
      boolFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    boolFuncProps->countOnesFunction = (int)value;
  }else if(strcmp(type,"combinedParityCOorMPFunction")==0){
    if((int)value){
      boolFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    boolFuncProps->combinedParityCOorMPFunction = (int)value;
  }else if(strcmp(type,"carryFunction")==0){
    if((int)value){
      boolFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    boolFuncProps->carryFunction = (int)value;
  }else if(strcmp(type,"classifierFunction")==0){
    if((int)value){
      boolFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    boolFuncProps->classifierFunction = (int)value;
  }else if(strcmp(type,"addNoiseToAction")==0){
    boolFuncProps->addNoiseToAction = value;
  }else if(strcmp(type,"actionNoiseMu")==0){
    boolFuncProps->actionNoiseMu = value;
  }else if(strcmp(type,"actionNoiseSigma")==0){
    boolFuncProps->actionNoiseSigma = value;
  }else if(strcmp(type,"samplingBias")==0){
    boolFuncProps->samplingBias = value;
  }else if(strcmp(type,"paymentRange")==0){
    printf("Payment Range is set internally!\n");
    /*boolFuncProps->paymentRange = (int)value;*/
  }else if(strcmp(type,"conditionLength")==0){
    boolFuncProps->conditionLength = (int)value;
  }else if(strcmp(type,"paritySize")==0){
    boolFuncProps->paritySize = (int)value;
  }else if(strcmp(type,"multiplexerBits")==0){
    boolFuncProps->multiplexerBits = (int)value;
    if(boolFuncProps->payoffLandscape) { /* redetermine payment range if set */
      if(boolFuncProps->multiplexerFunction)
	boolFuncProps->paymentRange = (1<<((boolFuncProps->multiplexerBits)+1))*100+200;
      else if(boolFuncProps->concatenatedMultiplexer)
	boolFuncProps->paymentRange = 1000*(boolFuncProps->conditionLength)/
	  (boolFuncProps->multiplexerBits + (1<<boolFuncProps->multiplexerBits)); 
    }
  }else if(strcmp(type,"payoffLandscape")==0){
    boolFuncProps->payoffLandscape = (int)value;
    if(boolFuncProps->payoffLandscape) { /* redetermine payment range if set */
      if(boolFuncProps->multiplexerFunction)
	boolFuncProps->paymentRange = (1<<((boolFuncProps->multiplexerBits)+1))*100+200;
      else if(boolFuncProps->concatenatedMultiplexer)
	boolFuncProps->paymentRange = 1000*(boolFuncProps->conditionLength)/
	  (boolFuncProps->multiplexerBits + (1<<boolFuncProps->multiplexerBits)); 
    }
  }else if(strcmp(type,"bmpX")==0){
    boolFuncProps->bmpX = (int)value;
  }else if(strcmp(type,"bmpY")==0){
    boolFuncProps->bmpY = (int)value;
  }else if(strcmp(type,"countOnesSize")==0){
    boolFuncProps->countOnesSize = (int)value;
  }else if(strcmp(type,"countOnesType")==0){
    boolFuncProps->countOnesType = (int)value;
  }else if(strcmp(type,"combinedType")==0){
    boolFuncProps->combinedType = (int)value;
  }else if(strcmp(type,"carrySize")==0){
    boolFuncProps->carrySize = (int)value;
  }else if(strcmp(type,"classifierFunctionSize")==0){
    boolFuncProps->classifierFunctionSize = (int)value;
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
  free(boolFuncProps);
  return;
}

/**
 * Function meaningless for the boolean function case because all are problem 
 * instances are equally probable
 */
int do10FoldedCrossValidation(long seed)
{
  return 1;
}

/**
 * Function meaningless for testing because all remains the same.
 */
int doTesting(int doIt)
{
  bfeTestState=0;
  return 1;
}

int nextTestState(struct attributes *state)
{
  int i,temp;
  if(bfeTestState >= (int)(1<<boolFuncProps->conditionLength)) {
    bfeTestState=0;
    return 0;
  }
  temp = bfeTestState;
  for(i=boolFuncProps->conditionLength-1; i>=0; i--) {
    if(temp%2 == 0) {
      state[i].bool = '0';
    }else{
      state[i].bool = '1';
    }
    temp = (int)(temp/2);
  }

  bfeTestState++;
  return 1;
}

int getCrossPoints(int *pos)
{
  if(! boolFuncProps->combinedParityCOorMPFunction) {
    return 0;
  }
  return boolFuncProps->paritySize;
}
