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
/     The default environment header - the specified functions and Macros need to be implemented in any environment/problem, XCS is applied to.
*/

#include "xcsMacros.h"

int isMultiStep();  //just return 1
int doNeedInputFile(); //just return 0
int setEnvParam(char *type, double value); //already excluded from xcs.c
int getConditionLength(); //input length

int getGeneralType(); /* this should return the problem type: 0 = boolean problem, 1 = integers, 2 = real, 3 = nominals, 4 = mixed */		//always return real
int getAttributeType(int att);
double getUpperBound(int att);
double getLowerBound(int att);

int getPaymentRange();
int getNumberOfActions();
double doAction(struct attributes *state, int action, int *correct);
void resetState(struct attributes *state);

double getExactEstimate(struct condAttribute *conArray, int class, double *mad);
int getNextNiche(struct attributes *state, int type);

int initEnv(FILE *fp);		//ignore
void fprintEnv(FILE *outfile);	//ignore
void freeEnv(); //delete environment
int do10FoldedCrossValidation(long seed); /* sets 10-folded crossvalidation test */	//return 0
int doTesting(int doIt); /* determines if test set should be presented */		//return 0
int nextTestState(struct attributes *state);						//return 0
int getCrossPoints(int *pos);								//return 0
