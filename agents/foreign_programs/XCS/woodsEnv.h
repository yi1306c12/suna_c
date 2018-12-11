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
/     The header of the woods environment coded binary.
/     The WOODS_LENGTH_OF_ONE_ATTRIBUTE macro specifies with how many bits one attribute 
/     in the environment is coded.
*/

#define WOODS_LENGTH_OF_ONE_ATTRIBUTE 3
#define WOODS_CONDITION_LENGTH (8*WOODS_LENGTH_OF_ONE_ATTRIBUTE+RANDOMBIT_EXISTS)
#define WOODS_NUMBER_OF_ACTIONS 8

#define WOODS_PAYMENT_RANGE 1000

#define WOODS_ADD_NOISE_TO_ACTION 0 /* codes binary which actions should have noise (0<=x<=256) */
#define WOODS_MU 0.0 /* the skew if the payoff noise */
#define WOODS_SIGMA 200.0 /* standard deviation of the normal distributed payoff noise */

#define ENEMY_EXISTS 0 /* if one, another agent is simulated that moves randomly in the maze */
#define RANDOMBIT_EXISTS 0 /* specifies if another random bit should be added to a sensory state */
#define SLIPPROB 0.0 /* the probability of moving to the cell next to the intended one if the floor is slippery at the current position */
#define RESET_ON_REWARD 1 /* resests when food is encountered */

/* mapping of sensors */

#define FREE0 '0'
#define FREE1 '0'
#define FREE2 '0'

#define FOODF0 '1'
#define FOODF1 '1'
#define FOODF2 '0'
#define FOODG0 '1'
#define FOODG1 '1'
#define FOODG2 '1'

#define OBSTACLEO0 '0'
#define OBSTACLEO1 '1'
#define OBSTACLEO2 '0'
#define OBSTACLEQ0 '0'
#define OBSTACLEQ1 '1'
#define OBSTACLEQ2 '1'

#define ENEMY0 '1'
#define ENEMY1 '0'
#define ENEMY2 '1'

#include "env.h"

int isMultiStep(){return 1;}
int doNeedInputFile(){return 1;}

/* local functions */

/* action related stuff */
int moveObject(int action,int *x,int *y);

/* Movements in all eight directions */
int north(int *x,int *y);
int south(int *x,int *y);
int west(int *x,int *y);
int east(int *x,int *y);
int northeast(int *x,int *y);
int southeast(int *x,int *y);
int southwest(int *x,int *y);
int northwest(int *x,int *y);

/* get the reward and set the new state */
double checkReward(int x,int y);
void setState(char *state);

/* a reset has to be called if reward was received */
void resetState(char *state);
void setStartPos();
void setFreeRandPos(int *x,int *y);

/* an 'enemy'(demon) in the environment */
int moveEnemy();
void hideEnemy();
void showEnemy();

/* get properties for testing purposes */
int getEnvXSize();
int getEnvYSize();
int getXPos();
int getYPos();
int setPos(int x,int y,char *state);

char getRandomChar();

double getExactEstimate(char *conArray, int class, double *mad){return 0.;}

int do10FoldedCrossValidation() {return 0;} /* sets 10-folded crossvalidation test */
int doTesting(int doIt) {return 0;} /* determines if test set should be presented */
int nextTestState(struct attributes *state) {return 0;}
int getCrossPoints(int *pos) {return 0;}
