/*
 *      Created: 2014
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "General_Problem.h"


Reinforcement_Learning* current_env;
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

void setup(Reinforcement_Environment* env)
{
	current_env= env;
}

int initEnv(FILE *fp)
{

}

/**
 * Free the allocated memory.
 */
void freeEnv()
{

}

/**
 * Returns the number of actions. 
 */
int getNumberOfActions()
{
	return current_env->number_of_action_vars;
}

/**
 * Returns the lenght of a problem instance.
 */
int getConditionLength()
{
	return current_env->number_of_observation_vars;
}

/**
 * Returns the type (boolean, int, real, nominal) 
 */
int getAttributeType(int att)
{
	return 2;
}

/**
 * Returns the upper bound of an attribute.
 */
double getUpperBound(int att)
{
	printf("Asked about upper bound\n");
	exit(1);
	return 0;
}

/**
 * Returns the lower bound of an attribute.
 */
double getLowerBound(int att)
{
	printf("Asked about lower bound\n");
	exit(1);
	return 0;
}

/**
 * Returns the payment range of this problem (i.e. the maximum payoff).
 */
int getPaymentRange()
{
	return MAXIMUM_REWARD;
}

/**
 * Resets the state to a new problem instance.
 * state has memory allocated to support one complete problem instance.
 */ 
void resetState(struct attributes *state)
{

}

/** 
 	Act on the Environment

	*reset=1 when problem finishes and 0 otherwise
	act is the action number (the output is always considered an integer)
	attributes are the observation variables to be set by 
 */ 
double doAction(struct attributes *ss, int act, int *reset)
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
	return 0;
}

/**
 * Returns the exact estimate for the given condition and action
 */
double getExactEstimate(struct condAttribute *conArray, int class, double *mad)
{
  return  0;
}


/**
 * Print the environment constants to file.
 */
void fprintEnv(FILE *outfile)
{


}

/**
 * Tries to set parameter 'type' to value 'value. If 'type' does not exist, 0 is returned.
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
  }else{
    printf("%s not supported in current environment\n",type);
    return 0;
  }
  return 1;
}
 */

/**
 * Prepares the data for stratified 10-folded cross validation
 */
int do10FoldedCrossValidation(long seed)
{
  return 0;
}


/**
 * Sets the system to test mode and back to normal mode dependent on doIt.
 * @return Returns the size of the learning set / test set
 */
int doTesting(int doIt)
{
  return 0;
}

/** 
 * Specifies the next test problem in state. 
 * Returns if there is another test problem.
 * If there is none, returns zero and resets the test problems. 
 * If not in test mode, returns zero and does nothing
 */
int nextTestState(struct attributes *state)
{
  return 0;
}

