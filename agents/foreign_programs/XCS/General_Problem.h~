/*
/       (XCS-C 1.2)
/	------------------------------------
/	Learning Classifier System based on accuracy
/
/     by Martin Butz
/     University of Wuerzburg / University of Illinois at Urbana/Champaign
/     butz@illigal.ge.uiuc.edu
/     Last modified: 12-02-2003
/
/     The environment header for testing on an .arff data file.
*/

#include "xcsMacros.h"

#define ARFF_ADD_NOISE_TO_ACTION 0
#define ARFF_ACTION_NOISE_MU 0
#define ARFF_ACTION_NOISE_SIGMA 0
#define ARFF_PAYMENT_RANGE 1000

struct arffFuncProps{
  double addNoiseToAction; /* if noise should be added to the action */
  double actionNoiseMu; /* the mean of the noise */
  double actionNoiseSigma; /* the standard deviation of the noise */
  double paymentRange; /* the payment range */
};

struct dataAttributeType{
  int type;
  char bool;
  int lowerBound;
  int upperBound;
  double lowerRealBound;
  double upperRealBound;
  char **nominals;
};

struct dataAttribute{
  int type;
  char bool;
  int integer;
  double real;
};

int isMultiStep() {return 0;}
int doNeedInputFile(){return 1;}

int setEnvParam(char *type, double value);

int getConditionLength();
int getGeneralType() {return 4;} /* this should return the problem type: 0 = boolean problem, 1 = integers, 2 = real, 3 = nominals, 4 = mixed */
int getAttributeType(int att);
double getUpperBound(int att);
double getLowerBound(int att);

int getPaymentRange();
int getNumberOfActions();
double doAction(struct attributes *state, int action, int *correct);
void resetState(struct attributes *state);

int initEnv(FILE *fp);
void freeEnv();
void fprintEnv(FILE *outfile);

double getExactEstimate(struct condAttribute *conArray, int class, double *mad);
int getNextNiche(struct attributes *state, int type);

/* from env.h */
int do10FoldedCrossValidation(long seed); /* sets 10-folded crossvalidation test */
int doTesting(); /* determines if test set should be presented */
int nextTestState(struct attributes *state);

int getCrossPoints(int *pos) {return 0;}
