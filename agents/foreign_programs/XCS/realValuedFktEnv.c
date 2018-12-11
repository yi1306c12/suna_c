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
#include "realValuedFktEnv.h"
#include "xcsMacros.h"

/* the current properties of this boolean function */
struct realEnv *realFuncProps = 0;
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
  return realFuncProps->conditionLength;
}

/**
 * Returns the type (int, boolean, real) for the specified type.
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
  return 1.; /* real case we go from 0 to 1 */
}

/**
 * Returns the lower bound of an attribute.
 */
double getLowerBound(int att)
{
  return 0; /* real case we go from 0 to 1 */
}

/**
 * Returns the payment range of this problem (i.e. the maximum payoff).
 */
int getPaymentRange()
{
  return realFuncProps->paymentRange;
}

/**
 * Returns the number of possible actions.
 */
int getNumberOfActions()
{
  if(realFuncProps->concatenatedMultiplexer==1)
    return (int)(1<<(int)((realFuncProps->conditionLength)/(realFuncProps->multiplexerBits + (1<<realFuncProps->multiplexerBits))));
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
    for(i=0; i < realFuncProps->conditionLength; i++) {
      state[i].real = (double) urand();
      state[i].type = getGeneralType();
    }
    if(realFuncProps->samplingBias == 0.5) {
      redo = 0;
    }else{
      doAction(state, action, &correct);
      /* accept a string with result '0' with probability samplingBias */
      redo=0;
      if(correct==1) {
	if(urand() < realFuncProps->samplingBias)
	  redo = 1;
      }else{
	if(urand() > realFuncProps->samplingBias)
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
    if(ss[i].real < 0.5)
      state[i] = '0';
    else
      state[i] = '1';
  }
  
  /* return the perceived reward */
  if(realFuncProps->constantFunction) {
    *correct = 1;
    value = (double)realFuncProps->paymentRange;
  }else if(realFuncProps->randomFunction)
    value = doRandomAction(state,act,correct);
  else if(realFuncProps->parityFunction)
    value = doParityAction(state,act,correct);
  else if(realFuncProps->multiplexerFunction)
    value = doMPAction(state,act,correct);
  else if(realFuncProps->concatenatedMultiplexer)
    value = doConcatenatedMultiplexerAction(state, act, correct);
  else if(realFuncProps->biasedMultiplexer)
    value = doBiasedMultiplexerAction(state,act,correct);
  else if(realFuncProps->countOnesFunction)
    value = doCountOnesAction(state, act, correct);
  else if(realFuncProps->carryFunction)
    value = doCarryAction(state, act, correct);
  else if(realFuncProps->classifierFunction)
    value = doClassifierAction(state, act, correct);
  else{
    value = 0;
    printf("Some function should have been chosen in env.h or set in the input file!\n");
  }

  if(realFuncProps->addNoiseToAction>0) {
    if(realFuncProps->addNoiseToAction<4) { /* Add Gaussian noise to outcome value */
      if(  realFuncProps->addNoiseToAction==1 || (realFuncProps->addNoiseToAction==2 && act==0) || (realFuncProps->addNoiseToAction==3 && act==1))
	value += (realFuncProps->actionNoiseMu) + nrand() * (realFuncProps->actionNoiseSigma);
    }else{ /* Alternate outcome with a certain probabilitiy possibly action dependent */
      if(  realFuncProps->addNoiseToAction<=5 || (realFuncProps->addNoiseToAction<=6 && act==0) || (realFuncProps->addNoiseToAction>6 && realFuncProps->addNoiseToAction<=7 && act==1)) {
	if(urand() < realFuncProps->addNoiseToAction-(double)(int)(realFuncProps->addNoiseToAction)) {
	  if(value == realFuncProps->paymentRange)
	    value = 0;
	  else
	    value = realFuncProps->paymentRange;
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
  return (double)((int)(((double) rand() / (RAND_MAX+1.0)) *2) * (realFuncProps->paymentRange));
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

  j=realFuncProps->carrySize;
  for(i=0; i<realFuncProps->carrySize; i++,j++) {
    if(state[i]=='0' && state[j]=='0') {
      break;
    }else if(state[i]=='1' && state[j]=='1') {
      carry=1;
      break;
    }
  }
  
  if(act==carry) {
    *correct=1;
    return realFuncProps->paymentRange;
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
  for(i=0, j=0; i<realFuncProps->countOnesSize; i++){
    if(state[i]=='1')/* count the number of ones */
      j++;
  }
  switch(realFuncProps->countOnesType){
  case 0: /* the number of ones determines the outcome */
    if(j*2 >= realFuncProps->countOnesSize){ /* this is the action one */
      if(act==1){
	*correct = 1;
	return realFuncProps->paymentRange;
      }else{
	*correct = 0;
	return 0;
      }
    }else { /* If equal or less, the action should be zero! */
      if(act==0){
	*correct = 1;
	return realFuncProps->paymentRange;
      }else{
	*correct = 0;
	return 0;
      }
    }
    break;
  case 1: /* The number of ones determines the payoff level returned */
    if(j*2 > realFuncProps->countOnesSize){ /* this is the action one */
      if(act==1) {
	*correct = 1;
	return (realFuncProps->paymentRange) * j / (realFuncProps->countOnesSize);
      }else{
	*correct = 0;
	return (realFuncProps->paymentRange) * ((realFuncProps->countOnesSize)-j) / (realFuncProps->countOnesSize);
      }
    }else if(j*2 == realFuncProps->countOnesSize) { /* 50/50 here - with this it is not possible to distinguish between action - so either action is correct!*/
      *correct = 1;
      return (realFuncProps->paymentRange)/2;
    }else{
      if(act==0){
	*correct = 1;
	return (realFuncProps->paymentRange)*((realFuncProps->countOnesSize)-j) / (realFuncProps->countOnesSize);
      }else{
	*correct = 0;
	return (realFuncProps->paymentRange) * j / (realFuncProps->countOnesSize);
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
  for(i=0; i<realFuncProps->paritySize; i++){
    if(state[i]=='1')
      sum++;
  }
  if(sum%2==0)
    if(act==0){
      *correct =1;
      return realFuncProps->paymentRange;
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
      return realFuncProps->paymentRange;
    }
}

/**
 * Multiplexer Function
 */
double doMPAction(char *state, int act, int *correct)
{
  int place=realFuncProps->multiplexerBits;
  int i;
  double reward;
 
  /* get the place of the by the first index bits referenced spot */
  for(i=0,place=realFuncProps->multiplexerBits; i<realFuncProps->multiplexerBits; i++) {
    if(state[i]=='1')
      place += (int)(1<<((realFuncProps->multiplexerBits)-1-i));
  }
  /* determine the corresponding reward and set 'correct' */
  if((act==1 && state[place]=='1') || (act==0 && state[place]=='0')) {
    /* the correct classification was chosen */
    *correct=1;
    if(realFuncProps->payoffLandscape)
      reward= 300.+(double)(((place-(realFuncProps->multiplexerBits))*200)+100*(int)(state[place]-'0'));
    else
      reward = realFuncProps->paymentRange;
  }else{
    /* the incorrect classification was chosen */
    *correct=0;
    if(realFuncProps->payoffLandscape)
      reward= 0.+(double)(((place - (realFuncProps->multiplexerBits))*200)+100*(int)(state[place]-'0'));
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

  for(i=0,place=0; i<realFuncProps->bmpX; i++){
    if(state[i]=='1')
      place += (int)(1<<(realFuncProps->bmpX-i-1));
  }
  if(place >= ((int)(1<<(realFuncProps->bmpX)))/2)
    zeroone=1;
  increment=((int)(1<<(realFuncProps->bmpY))) - 1 + realFuncProps->bmpY;
  for(placelocal=0, i=0; i< realFuncProps->bmpY; i++){
    if(state[realFuncProps->bmpX + place * increment + i]=='1')
      placelocal += (int)(1<<(realFuncProps->bmpY-i-1));
  }
  if(zeroone==0){
    if(placelocal==0){
      action='0';
    }else{
      placelocal--;
      action=state[realFuncProps->bmpX + place*increment + realFuncProps->bmpY + placelocal];
    }
  }else{
    if(placelocal == ((int)(1<<realFuncProps->bmpY))-1){
      action='1';
    }else{
      action=state[realFuncProps->bmpX + place*increment + realFuncProps->bmpY + placelocal];
    }
  }
  /* printf("%s -> %c\n",state,action);*/
 if((action=='1'&& act==1) || (action=='0' && act==0)){
    *correct=1;
    return realFuncProps->paymentRange;
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
  int place=realFuncProps->multiplexerBits;
  int nrMPs = (int)((realFuncProps->conditionLength)/(realFuncProps->multiplexerBits + (1<<realFuncProps->multiplexerBits)));
  int length = realFuncProps->multiplexerBits + (1<<realFuncProps->multiplexerBits);
  int i,j, nOne;
  double reward;
 
  *correct=1;
  reward=0;
  nOne=0;
  for(i=0; i<nrMPs; i++) {
    /* get the place of the spot referenced by the first index bits */
    for(j=0, place=length*i+realFuncProps->multiplexerBits; j < realFuncProps->multiplexerBits; j++) {
      if(state[length*i+j]=='1')
	place += (int)(1<<((realFuncProps->multiplexerBits)-1-j));
    }
    /* determine the corresponding reward and set 'correct' */
    if(realFuncProps->concatenatedMultiplexer==1) {
      if((act==1 && state[place]=='1') || (act==0 && state[place]=='0')) {
	/* the right action was chosen */
	if(realFuncProps->payoffLandscape)
	  reward += 1000;
      }else{
	/* the wrong action was chosen */
	*correct=0;
	if(!realFuncProps->payoffLandscape)
	  break; /* if one is wrong and no payoff Landscape, all is wrong */
      }
    }else{
      if(state[place]=='1') {
	nOne++;
      }
    }
  }
  if(realFuncProps->concatenatedMultiplexer==2) {
    if((nOne>1 && act==1) || (nOne<2 && act==0)) {/*type with only one action*/
      /*printf("Is correct %s - %s: %d\n",state, act, nOne);*/
      *correct = 1;
      reward = 1000;
    }else{
      *correct = 0;
      reward = 0;
    }
    if(realFuncProps->payoffLandscape) {
      if(act==1)
	reward = 1000 * nOne;
      else
	reward = 1000 * (nrMPs-nOne);
    }
  }else{
    if(*correct && !realFuncProps->payoffLandscape)
      reward = 1000;
    /* printf("%s with act %s was assigned %d with reward %f\n",state, act, *correct, reward);*/
  }
  return reward;
}




int getNextNiche(struct attributes *state)
{
  int num, i, j, k, l, move;
  
  if(realFuncProps->multiplexerFunction) {
    /* Multiplexer Function */
    num = (int)(1<<(realFuncProps->multiplexerBits + 1));
    if(num <= nicheState) /* no more niches */
      return 0;
    
    /* more niches to come... */
    j=nicheState;
    for(i=0; i<realFuncProps->multiplexerBits; i++) {
      (state[i]).type = 2;
      if( j >= (int)(1<<(realFuncProps->multiplexerBits-i))) {
	(state[i]).real=1.;
	j -= (int)(1<<(realFuncProps->multiplexerBits-i));
      }else{
	(state[i]).real=0.;
      }
    }
    for(j=0; j<(1<<(realFuncProps->multiplexerBits)); j++) {
      (state[realFuncProps->multiplexerBits+j]).type = 0;
      (state[realFuncProps->multiplexerBits+j]).bool = DONT_CARE;
    }
    j = nicheState/2;
    (state[realFuncProps->multiplexerBits+j]).type = 2;
    (state[realFuncProps->multiplexerBits+j]).real = 0.+(nicheState%2);
    nicheState++;
    return 1;
  }else if(realFuncProps->countOnesFunction) {
    if(realFuncProps->countOnesType==1) {
      /* Count ones problem type 1 */
      num = (int)(1<<realFuncProps->countOnesSize);
      if(num <= nicheState)
	return 0;
      
      /* more niches to come... */
      for(i=0; i<realFuncProps->conditionLength; i++) {
	(state[i]).type = 0;
	(state[i]).bool=DONT_CARE;
      }
      j=nicheState;
      for(i=0; i<realFuncProps->countOnesSize; i++) {
	if( j >= (int)(1<<(realFuncProps->countOnesSize-i-1))) {
	  (state[i]).type = 2;
	  (state[i]).real=1.;
	  j -= (int)(1<<(realFuncProps->multiplexerBits-i-1));
	}else{
	  (state[i]).type = 2;
	  (state[i]).real=0.;
	}
      }
      nicheState++;
      return 1;
    }else{
      /* Count ones problem type 0 */
      num = 2 * NChooseK(realFuncProps->countOnesSize, (realFuncProps->countOnesSize-1)/2 );
      if(num <= nicheState)
	return 0;

      /* MORE NICHES TO COME */
      for(i=0; i<realFuncProps->conditionLength; i++) {
	(state[i]).type = 0;
	(state[i]).real=DONT_CARE;
      }
      /* zero or one niche */
      if(nicheState < num/2) {
	(state[0]).type = 2;
	(state[0]).real=0.;
	j=nicheState;
      }else{
	(state[0]).type = 2;
	(state[0]).real=1.;
	j=nicheState - num/2;
      }
      /* need to generate more than half to be correct */
      for(i=1; i<(realFuncProps->countOnesSize+1)/2; i++) {
	(state[i]).type = 2;
	(state[i]).real=(state[i-1]).real;
      }

      /* iteratively generate actual test niche */
      for(k=0; k<j; k++) {
	if((state[realFuncProps->countOnesSize-1]).type == 0) {
	  /* just move last entry */
	  for( i = 1; i<realFuncProps->countOnesSize ; i++) {
	    if((state[realFuncProps->countOnesSize-1-i]).type != 0){
	      (state[realFuncProps->countOnesSize-i]).type = 2;
	      (state[realFuncProps->countOnesSize-i]).real = (state[realFuncProps->countOnesSize-1-i]).real;
	      (state[realFuncProps->countOnesSize-1-i]).type = 0;
	      (state[realFuncProps->countOnesSize-1-i]).real = DONT_CARE;
	      break; 
	    }
	  }
	}else{
	  /* move chunk to next state */
	  (state[realFuncProps->countOnesSize-1]).type = 0;
	  (state[realFuncProps->countOnesSize-1]).bool = DONT_CARE;
	  /* detect chunk to be moved */
	  for( i = 1; i<realFuncProps->countOnesSize; i++) {
	    if( (state[realFuncProps->countOnesSize-1-i]).type == 0) {
	      move = i;
	      if(move == (realFuncProps->countOnesSize+1)/2 ){
		printf("Seems to be done!?\n");
		return 0;
	      }
	      break;
	    }else{
	      (state[realFuncProps->countOnesSize-1-i]).type = 0;
	      (state[realFuncProps->countOnesSize-1-i]).bool = DONT_CARE;
	    }
	  }
	  /* move specifies the number of guys that will be moved back */
	  /* look for place where to move it */
	  for(i++; i<realFuncProps->countOnesSize; i++) {
	    if((state[realFuncProps->countOnesSize-1-i]).type != 0) {
	      /* found it */
	      (state[realFuncProps->countOnesSize-i]).type = 2;
	      (state[realFuncProps->countOnesSize-i]).real=(state[realFuncProps->countOnesSize-1-i]).real;
	      (state[realFuncProps->countOnesSize-1-i]).type = 0;
	      (state[realFuncProps->countOnesSize-1-i]).bool = DONT_CARE;
	      for(l=0; l<move; l++) {
		(state[realFuncProps->countOnesSize-i+1+l]).type = 2;
		(state[realFuncProps->countOnesSize-i+1+l]).real = (state[realFuncProps->countOnesSize-i+l]).real;
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
  }else if(realFuncProps->carryFunction) {
    /* MORE NICHES TO COME */
    for(i=0; i<realFuncProps->conditionLength; i++) {
      (state[i]).type = 0;
      (state[i]).bool = DONT_CARE;
    }
    i=0;
    j=realFuncProps->carrySize;
    
    if(nicheState+1 < (int)(1<<(realFuncProps->carrySize))) {
      (state[i]).type = 2;
      (state[i]).real=1.;
      (state[j]).type = 2;
      (state[j]).real=1.;
      for(k=0; k<nicheState; k++) {
	(state[i]).type = 2;
	(state[i]).real=1.;
	(state[j]).type = 0;
	(state[j]).bool = DONT_CARE;
	if(i+1 < realFuncProps->carrySize) {
	  /* set the next position to both '1' so that there is a carry */
	  i++;
	  j++;
	  (state[i]).type = 2;
	  (state[i]).real=1.;
	  (state[j]).type = 2;
	  (state[j]).real=1.;
	}else{
	  (state[i]).type = 0;
	  (state[i]).bool = DONT_CARE;
	  (state[j]).type = 0;
	  (state[j]).bool = DONT_CARE;
	  i--;
	  j--;
	  /* move back to the one where (state[i]).real=='1' and switch */
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
	    (state[j]).real=1.;
	  }else{
	    printf("We should not arrive here!!!\n");
	    return 0;
	  }
	  i++;
	  j++;
	  (state[i]).type = 2;
	  (state[i]).real=1.;
	  (state[j]).type = 2;
	  (state[j]).real=1.;
	}
      }
    }else{
      /* make sure there is no carry */
      (state[i]).type = 2;
      (state[i]).real=0;
      (state[j]).type = 2;
      (state[j]).real=0;
      for(k=0; k<nicheState+1-(int)(1<<(realFuncProps->carrySize)); k++) {
	if((state[i]).type == 2 && (state[j]).type == 2) {
	  (state[i]).type = 2;
	  (state[i]).real=0;
	  (state[j]).type = 0;
	  (state[j]).real=DONT_CARE;
	  if(i+2 < realFuncProps->carrySize) {
	    /* set the next position to both '1' so that there is a carry */
	    i++;
	    j++;
	    (state[i]).type = 2;
	    (state[i]).real=0;
	    (state[j]).type = 2;
	    (state[j]).real=0;
	  }else if(i+1 < realFuncProps->carrySize) {
	    i++;
	    j++;
	    (state[i]).type = 2;
	    (state[i]).real=0;
	    (state[j]).type = 0;
	    (state[j]).real=DONT_CARE;
	  }
	}else{
	  /* move back */
	  while((state[i]).real==DONT_CARE && i>=0) {
	    (state[i]).type = 0;
	    (state[i]).real=DONT_CARE;
	    (state[j]).type = 0;
	    (state[j]).real=DONT_CARE;
	    i--;
	    j--;
	  }
	  if(i>=0) {
	    (state[i]).type = 0;
	    (state[i]).real=DONT_CARE;
	    (state[j]).type = 2;
	    (state[j]).real=0;
	  }else{
	    return 0;
	  }
	  if(i+2 < realFuncProps->carrySize) {
	    /* set the next position to both '0' so that there is no carry */
	    i++;
	    j++;
	    (state[i]).type = 2;
	    (state[i]).real=0;
	    (state[j]).type = 2;
	    (state[j]).real=0;
	  }else if(i+1 < realFuncProps->carrySize) {
	    i++;
	    j++;
	    (state[i]).type = 2;
	    (state[i]).real=0;
	    (state[j]).type = 0;
	    (state[j]).real=DONT_CARE;
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
  if(realFuncProps->multiplexerFunction) {
    /* determine all possible outcome positions */
    av = getAveragedMPOutcomes(0, 0, conArray, class);
    if(realFuncProps->payoffLandscape) {
      *mad = getMADMPOutcomes(0, 0, conArray, class, av);
    }else{
      *mad = 2000. * (av/1000. - av/1000. * av/1000.);
    }
  }else if(realFuncProps->countOnesFunction) {
    if(realFuncProps->countOnesType==0) {
      av = getAveragedCO0Outcomes(conArray, class);
      *mad = 2000. * (av/1000. - av/1000. * av/1000.);
    }else{
      av = getAveragedCO1Outcomes(0, conArray, class);
      *mad = getMADCO1Outcomes(0, conArray, class, av);
    }
  }else if(realFuncProps->classifierFunction) {
    for(i=0; i<realFuncProps->classifierFunctionSize ; i++) {
      if(conArray[i]==DONT_CARE)
	break;
    }
    av = 500;
    *mad = 500.* (double)(realFuncProps->classifierFunctionSize-i)/(double)(realFuncProps->classifierFunctionSize);
    if(i==realFuncProps->classifierFunctionSize){
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
  for(i=0; i<realFuncProps->countOnesSize; i++) {
    if(conArray[i]=='1')
      numOnes++;
    else if(conArray[i]!='0')
      numDCs++;
  }
  numMissing = ((realFuncProps->countOnesSize + 1) / 2)-numOnes;
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

  if(i<realFuncProps->countOnesSize) {
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

  if(i<realFuncProps->countOnesSize) {
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
  if(i<realFuncProps->multiplexerBits) {
    if(conArray[i]=='0') {
      return getAveragedMPOutcomes(i+1, pos, conArray, class);
    }else if(conArray[i]=='1') {
      return getAveragedMPOutcomes(i+1, pos+(1<<(realFuncProps->multiplexerBits-i-1)), conArray, class);
    }else{
      conArray[i]='0';
      result = getAveragedMPOutcomes(i+1, pos, conArray, class);
      conArray[i]='1';
      result += getAveragedMPOutcomes(i+1, pos+(1<<(realFuncProps->multiplexerBits-i-1)), conArray, class);
      conArray[i]='#';
      return 0.5*result;
    }
  }
  pos += realFuncProps->multiplexerBits;
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
  if(i<realFuncProps->multiplexerBits) {
    if(conArray[i]=='0') {
      return getMADMPOutcomes(i+1, pos, conArray, class, average);
    }else if(conArray[i]=='1') {
      return getMADMPOutcomes(i+1, pos+(1<<(realFuncProps->multiplexerBits-i-1)), conArray, class, average);
    }else{
      conArray[i]='0';
      result = getMADMPOutcomes(i+1, pos, conArray, class, average);
      conArray[i]='1';
      result += getMADMPOutcomes(i+1, pos+(1<<(realFuncProps->multiplexerBits-i-1)), conArray, class, average);
      conArray[i]='#';
      return 0.5*result;
    }
  }
  pos += realFuncProps->multiplexerBits;
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
  assert((realFuncProps = (struct realEnv *) calloc(1,sizeof(struct realEnv)))!=0);

  realFuncProps->constantFunction = CONSTANT_FUNCTION;
  realFuncProps->randomFunction = RANDOM_FUNCTION;
  realFuncProps->parityFunction = PARITY_FUNCTION;
  realFuncProps->multiplexerFunction = MULTIPLEXER_FUNCTION;
  realFuncProps->concatenatedMultiplexer = CONCATENATED_MULTIPLEXER;
  realFuncProps->biasedMultiplexer = BIASED_MULTIPLEXER;
  realFuncProps->countOnesFunction = COUNT_ONES_FUNCTION;
  realFuncProps->carryFunction = CARRY_FUNCTION;
  realFuncProps->classifierFunction = CLASSIFIER_FUNCTION;

  realFuncProps->addNoiseToAction = ADD_NOISE_TO_ACTION;
  realFuncProps->actionNoiseMu = ACTION_NOISE_MU;
  realFuncProps->actionNoiseSigma = ACTION_NOISE_SIGMA;

  realFuncProps->samplingBias = SAMPLING_BIAS;

  realFuncProps->paymentRange = PAYMENT_RANGE;
  realFuncProps->conditionLength = CONDITION_LENGTH;

  realFuncProps->paritySize = PARITY_SIZE;

  realFuncProps->multiplexerBits = MULTIPLEXER_BITS;
  realFuncProps->payoffLandscape = PAYOFF_LANDSCAPE;

  realFuncProps->bmpX = BMP_X;
  realFuncProps->bmpY = BMP_Y;

  realFuncProps->countOnesSize = COUNT_ONES_SIZE;
  realFuncProps->countOnesType = COUNT_ONES_TYPE;

  realFuncProps->carrySize = CARRY_SIZE;

  realFuncProps->classifierFunctionSize = CLASSIFIER_FUNCTION_SIZE;
  return 1;
}

/**
 * Print the environment constants to file.
 */
void fprintEnv(FILE *outfile)
{
  fprintf(outfile, "# constantFunction %d; randomFunction %d; parityFunction %d; multiplexerFunction %d; concatenatedMP %d; biasedMP %d; countOnesFunction %d; carryFunction %d; classifierFunction %d; addNoiseToAction %f; actionNoiseMu %f; actionNoiseSigma %f; samplingBias %f;\n", realFuncProps->constantFunction, realFuncProps->randomFunction, realFuncProps->parityFunction, realFuncProps->multiplexerFunction, realFuncProps->concatenatedMultiplexer, realFuncProps->biasedMultiplexer, realFuncProps->countOnesFunction, realFuncProps->carryFunction, realFuncProps->classifierFunction, realFuncProps->addNoiseToAction, realFuncProps->actionNoiseMu, realFuncProps->actionNoiseSigma, realFuncProps->samplingBias); 
  fprintf(outfile, "# paymentRange %d; conditionLength %d; paritySize %d; multiplexerBits %d; payoffLandscape %d; bmpX %d; bmpY %d; countOnesSize %d; countOnesType %d; carrySize %d; classifierFunctionSize %d\n", realFuncProps->paymentRange, realFuncProps->conditionLength, realFuncProps->paritySize, realFuncProps->multiplexerBits, realFuncProps->payoffLandscape,realFuncProps->bmpX, realFuncProps->bmpY, realFuncProps->countOnesSize, realFuncProps->countOnesType, realFuncProps->carrySize, realFuncProps->classifierFunctionSize); 
}

/**
 * Resets the all function selections to zero.
 */
void newFunctionChoice()
{
  realFuncProps->constantFunction = 0;
  realFuncProps->randomFunction = 0;
  realFuncProps->multiplexerFunction = 0;
  realFuncProps->concatenatedMultiplexer = 0;
  realFuncProps->biasedMultiplexer = 0;
  realFuncProps->countOnesFunction = 0;
  realFuncProps->carryFunction = 0;
  realFuncProps->classifierFunction = 0;
}

/**
 * Tries to set parameter 'type' to value 'value. If 'type' does not exist, 0 is returned.
 */
int setEnvParam(char *type, double value)
{
  if(strcmp(type,"constantFunction")==0){
    if((int)value){
      realFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    realFuncProps->constantFunction = (int)value;
  }else if(strcmp(type,"randomFunction")==0){
    if((int)value){
      realFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    realFuncProps->randomFunction = (int)value;
  }else if(strcmp(type,"parityFunction")==0){
    if((int)value){
      realFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    realFuncProps->parityFunction = (int)value;
  }else if(strcmp(type,"multiplexerFunction")==0){
    if((int)value){
      realFuncProps->paymentRange = PAYMENT_RANGE;
      if(realFuncProps->payoffLandscape) /* redetermine payment range if set */
	realFuncProps->paymentRange = (1<<((realFuncProps->multiplexerBits)+1))*100+200;
      newFunctionChoice();
    }
    realFuncProps->multiplexerFunction = (int)value;
  }else if(strcmp(type,"concatenatedMultiplexer")==0) {
    if((int)value) {
      realFuncProps->paymentRange = PAYMENT_RANGE;
      if(realFuncProps->payoffLandscape) /* redetermine payment range if set */
	/* size of condition divided by size of one multiplexer */
	realFuncProps->paymentRange = 1000*(realFuncProps->conditionLength) / (realFuncProps->multiplexerBits + (1<<realFuncProps->multiplexerBits)); 
      newFunctionChoice();
    }
    realFuncProps->concatenatedMultiplexer = (int)value;
  }else if(strcmp(type,"biasedMultiplexer")==0){
    if((int)value) {
      realFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    realFuncProps->biasedMultiplexer = (int)value;
  }else if(strcmp(type,"countOnesFunction")==0){
    if((int)value){
      realFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    realFuncProps->countOnesFunction = (int)value;
  }else if(strcmp(type,"carryFunction")==0){
    if((int)value){
      realFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    realFuncProps->carryFunction = (int)value;
  }else if(strcmp(type,"classifierFunction")==0){
    if((int)value){
      realFuncProps->paymentRange = PAYMENT_RANGE;
      newFunctionChoice();
    }
    realFuncProps->classifierFunction = (int)value;
  }else if(strcmp(type,"addNoiseToAction")==0){
    realFuncProps->addNoiseToAction = value;
  }else if(strcmp(type,"actionNoiseMu")==0){
    realFuncProps->actionNoiseMu = value;
  }else if(strcmp(type,"actionNoiseSigma")==0){
    realFuncProps->actionNoiseSigma = value;
  }else if(strcmp(type,"samplingBias")==0){
    realFuncProps->samplingBias = value;
  }else if(strcmp(type,"paymentRange")==0){
    printf("Payment Range is set internally!\n");
    /*realFuncProps->paymentRange = (int)value;*/
  }else if(strcmp(type,"conditionLength")==0){
    realFuncProps->conditionLength = (int)value;
  }else if(strcmp(type,"paritySize")==0){
    realFuncProps->paritySize = (int)value;
  }else if(strcmp(type,"multiplexerBits")==0){
    realFuncProps->multiplexerBits = (int)value;
    if(realFuncProps->payoffLandscape) { /* redetermine payment range if set */
      if(realFuncProps->multiplexerFunction)
	realFuncProps->paymentRange = (1<<((realFuncProps->multiplexerBits)+1))*100+200;
      else if(realFuncProps->concatenatedMultiplexer)
	realFuncProps->paymentRange = 1000*(realFuncProps->conditionLength)/
	  (realFuncProps->multiplexerBits + (1<<realFuncProps->multiplexerBits)); 
    }
  }else if(strcmp(type,"payoffLandscape")==0){
    realFuncProps->payoffLandscape = (int)value;
    if(realFuncProps->payoffLandscape) { /* redetermine payment range if set */
      if(realFuncProps->multiplexerFunction)
	realFuncProps->paymentRange = (1<<((realFuncProps->multiplexerBits)+1))*100+200;
      else if(realFuncProps->concatenatedMultiplexer)
	realFuncProps->paymentRange = 1000*(realFuncProps->conditionLength)/
	  (realFuncProps->multiplexerBits + (1<<realFuncProps->multiplexerBits)); 
    }
  }else if(strcmp(type,"bmpX")==0){
    realFuncProps->bmpX = (int)value;
  }else if(strcmp(type,"bmpY")==0){
    realFuncProps->bmpY = (int)value;
  }else if(strcmp(type,"countOnesSize")==0){
    realFuncProps->countOnesSize = (int)value;
  }else if(strcmp(type,"countOnesType")==0){
    realFuncProps->countOnesType = (int)value;
  }else if(strcmp(type,"carrySize")==0){
    realFuncProps->carrySize = (int)value;
  }else if(strcmp(type,"classifierFunctionSize")==0){
    realFuncProps->classifierFunctionSize = (int)value;
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
  free(realFuncProps);
  return;
}
