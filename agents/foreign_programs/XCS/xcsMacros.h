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
/     The main header with all the Constants and possible selection of different procedures.
*/

#ifndef _XCS_MACROS_h_
#define _XCS_MACROS_h_


#define TAB_OUT_FILE "XCS_TEST" /* the output file for the results */

#define NR_EXPS 1 /* the number of experiments to take */
#define DO_10FOLDED_CROSS_VALIDATION 0 /* if 10-folded cross validation should be applied */
#define MAX_NR_STEPS 100000 /* the number of problems to solve in one experiment */
#define TEST_FREQUENCY 200 /* the frequency knowledge tests are executed */
#define POP_OUT_FREQUENCY 1000 /* the grequency of how often the population size should be saved */
#define DO_GENERATE_TREE 0 /* if tree should be generated and evaluated. Default=0 */

#define MAX_POP_SIZE 2000 /* the maximum size of the population */

#define ALPHA 0.1  /* default = 0.1; decreases the accuracy value if classifier's prediction error is */
		   /* higher than EPSILON_0*PAYMENT_RANGE.
		    * This constant results in a gap between accurate and not accurate classifiers */
#define BETA_P 0.2   /* default = 0.2; Learning Rate parameter of prediction */
#define BETA_E 0.2   /* default = 0.2; Learning Rate parameter of prediction error */
#define BETA_F 0.2   /* default = 0.2; Learning Rate parameter or fitness */
#define BETA_S 0.2   /* default = 0.2; Learning Rate parameter for action set size estimate */
#define GAMMA 0.95 /* default = 0.95; Discounting factor that discounts the maximum of the prediction array */
#define EPSILON_0 0.01 /* default = 0.01; TOLERATED ERROR in prediction error*/

#define NU 5. /* default = 5; exponent of the power function */
#define FALL_OFF_RATE 0.1 /* default = 0.1; ALPHA IN STEWARTS "CLASSIFIER FITNESS BASED ON ACCURACY", only relevant if SET_FITNESS_FUNCTION is set to 2 */

#define THETA_GA 25 /* default = 25; run GA if the average of the last GA application in the set is greater than THETA_GA */
#define FITNESS_REDUCTION 1 /* default = 1; reduce the fitness of the offspring classifier */
#define DO_SET_PRE_ERR_DIRECTLY 0 /* default = 0; sets prediction and error directly to the real value in this problem */
#define DO_INITIALIZE_P_E 3 /* default = 3; sets prediction and error to current reward and average prediction error (1=pre only, 2=err only, 3=both */
#define DO_FITNESS_ADJUSTMENT 0 /* default = 0; if on numerosity increase (due to subsumption or identical offspring) fitness should be adjusted */
#define TOURNAMENT_SIZE 0.4 /* percentage of action set that takes part in tournament; if set to zero, proportionate selection is used; if set greater than one, tournament with fixed size is used */
#define FORCE_DIFFERENT_IN_TOURNAMENT 1 /* the probability that the two participants must be different */
#define SELECT_TOLERANCE 0.0001 /* the tolerance with which classifier fitness/error is considered similar */
#define CROSSOVER_TYPE 2 /* default=2; 0=uniform, 1=one-point, 2=two-point */
#define CROSSPX 0.8 /* default = 0.8; the probability to do crossover */
#define MUTPMUE 0.04 /* default = 0.04; probability of mutating one bit */
#define MUTATE_RAND 0.5 /* the range of mutation for integer/real values */
#define DO_GENERALIZATION_MUTATION 0 /* default = 0; flag for only generalization in mutation */
#define DO_NICHE_MUTATION 0 /* default = 0; flag for applying niche mutation instead of a general mutation */
#define DO_ADJUST_MUTATION 0 /* defines if mutation should be adjusted to desired specificity */

#define DO_MAM 1 /* determines if Moyenne Adaptive Modifee should be applied 
		  * 1=apply to all, 2=apply to prediction and action set size estimate only */
#define DO_GRADIENT 1 /* determines if a gradient-based TD update should be used */
#define DO_RELATIVE_FITNESS 1 /* determines if relative fitness (or error if doGAErrorBasedSelect=1) should be used */
#define DO_GA_ERROR_BASED_SELECT 0 /* GA selection based on error */

#define DELTA 0.1 /* default = 0.1; used to enforce the deletion of low fitness classifiers (= second deletion method) once they reached DELETION_EXPERIENCE */
#define THETA_DEL 20 /* default = 20; The experience required to use second deletion method */

#define DELETION_TYPE 1 /* default = 1; 0 = random deletion, 1 = fitness and |[A]| bias, 2 = fitness bias only, 3 = |[A]| bias only ; 4 = error bias only; 5 = error and |[A]| bias */

#define DONT_CARE_PROB 0.6 /* Probability of choosing DONT_CARE in a newly generated classifier */
#define COVER_RAND 0.5 /* The range of covering in real or integer valued input */

#define DO_GA_SUBSUMPTION 1 /* default = 1; flag for applying GA subsumption */
#define DO_ACTION_SET_SUBSUMPTION 1 /* default = 1; flag for applying action set subsumption */
#define THETA_SUB 20 /* default = 20; the experience required before can be subsumed */

#define EXPLORE_PROB 1.0 /* default = 1; The probability of choosing a random action while learning */
#define TELETRANSPORTATION 50 /* possible teletransportation in multi step environments (trial ends if counter exeeds this number) */

/* The following parameters cannot be altered with the parameter file */

#define INITIALIZE_POPULATION 0 /* default = 0; Boolean to check if population should be initialized */

#define PRE_INI 10.0 /* default = 10; prediction initialization in new classifiers */
#define PREER_INI 0.0 /* default = 0; prediction error initialization in new classifiers */
#define FIT_INI 0.01 /* default = 0.01; fitness initialization in new classifiers */

#define DO_FITNESSUPDATE_FIRST 0 /* default = 0; flag detemines when the fitness is updated */
#define UPDATEORDER_PRED_PREDERROR 0 /* default = 0; determines if the prediction should be updated before the prediction error */
#define CALCULATE_SIGMA_ERROR 0 /* if the error of a classifier should be calcualate according to the standdard deviation or according to the average absolut error */
#define MAX_SUBSUME_NUMEROSITY 10000 /* the largest of number of identical classifiers allowed for subsuming */

#define DONT_CARE '#' /* the don't care symbol */

#define _M 2147483647     /* modulus of PMMLCG (default = 2147483647 = 2^31 - 1)*/
#define _A 16807          /* default = 16807*/

struct attributes{
  int type;
  char boole;
  int integer;
  double real;
};

struct condAttribute{
  int type;
  char boole;
  int lowerBound;
  int upperBound;
  double lowerRealBound;
  double upperRealBound;
};

double urand();
double nrand();

int setSeed(int newSeed);
int getSeed();

int getIntValue(char *string);
double getDoubleValue(char *string);
void randomize(void);

double absDouble(double value);
double NChooseK(double n, double k);

int doesMatchAttribute(struct condAttribute cond, struct attributes att);

#endif
