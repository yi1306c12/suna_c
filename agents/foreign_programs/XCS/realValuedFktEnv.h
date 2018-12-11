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
/     The Boolean Function environment - header - here the size of the mp function can be defined.
*/

#include "env.h"

struct realEnv{
  int constantFunction;
  int randomFunction;
  int parityFunction;
  int multiplexerFunction;
  int concatenatedMultiplexer;
  int biasedMultiplexer;
  int countOnesFunction;
  int carryFunction;
  int classifierFunction;

  double samplingBias;

  double addNoiseToAction;
  double actionNoiseMu;
  double actionNoiseSigma;

  int paymentRange;
  int conditionLength;

  int paritySize;

  int multiplexerBits;
  int payoffLandscape;

  int bmpX;
  int bmpY; 

  int countOnesSize;
  int countOnesType;

  int carrySize;

  int classifierFunctionSize;
};

#define CONSTANT_FUNCTION 0
#define RANDOM_FUNCTION 1
#define PARITY_FUNCTION 0
#define MULTIPLEXER_FUNCTION 0
#define CONCATENATED_MULTIPLEXER 0
#define BIASED_MULTIPLEXER 0
#define COUNT_ONES_FUNCTION 0
#define CARRY_FUNCTION 0
#define CLASSIFIER_FUNCTION 0 /* a function that provides reward only classifier relative to test convergence models */

#define ADD_NOISE_TO_ACTION 0 /* 0=add no noise 1=add noise to both actions 2=add noise to action 0 3=add noise to action 1 4.XX = probability of other outcome (given only PAYMENT_RANGE and 0 as possible outcomes) */
#define ACTION_NOISE_MU 0.0
#define ACTION_NOISE_SIGMA 0.0

#define SAMPLING_BIAS 0.5 /* 0.5 means: There is no sampling bias! */

#define PAYMENT_RANGE 1000.   /* always for multiplexer: 1000, 1800, 3400, 6600, 13000 */
#define CONDITION_LENGTH 20   /* for MP: 6 11 20 37 70 */

/* Parity Function Constants */
#define PARITY_SIZE 5

/* Multiplexer Function Constants */
#define MULTIPLEXER_BITS 4  /* Number of position bits */ 
#define PAYOFF_LANDSCAPE 0 /* 1=layered reward scheme */

/* Biased Multiplexer Function */
#define BMP_X 2
#define BMP_Y 2

/* Count Ones Function */
#define COUNT_ONES_SIZE 5
#define COUNT_ONES_TYPE 2

#define CARRY_SIZE 3

#define CLASSIFIER_FUNCTION_SIZE 5

#define IS_MULTI_STEP 0 /* Boolean to check if this is a multi-step environment */

int setEnvParam(char *type, double value);
int getConditionLength();
int getGeneralType() {return 2;}
int getAttributeType(int att);
double getUpperBound(int att);
double getLowerBound(int att);

int getPaymentRange();
int getNumberOfActions();
int isMultiStep();
int doNeedInputFile(){return 0;}

double getExactEstimate(char *conArray, int class, double *mad);
int getNextNiche(struct attributes *state);

double doAction(struct attributes *state, int action, int *correct);

double doRandomAction(char *state, int act, int *correct);
double doParityAction(char *state, int act, int *correct);
double doMPAction(char *state, int act,int *correct);
double doConcatenatedMultiplexerAction(char *state, int act, int *correct);
double doBiasedMultiplexerAction(char *state, int act, int *correct);
double doCountOnesAction(char *state, int act, int *correct);
double doCarryAction(char *state, int act, int *correct);
double doClassifierAction(char *state, int act, int *correct);

void resetState(struct attributes *state);
void newFunctionChoice();
int initEnv(FILE *fp);
void freeEnv();
void fprintEnv(FILE *outfile);

