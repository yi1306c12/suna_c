/*
/       (XCS-C 1.2)
/	------------------------------------
/	Learning Classifier System based on accuracy
/
/     by Martin Butz
/     University of Wuerzburg / University of Illinois at Urbana/Champaign
/     butz@illigal.ge.uiuc.edu
/     Last modified: 09-04-2003
/
/     Header for actions with xClassifier and xClassifierSet.
*/

/**
 * The conditon of a classifier is coded as a linked list which speeds up the matching process
 * significantely especially in large problems. 
 */

#ifndef _CLASSIFIER_LIST_h
#define _CLASSIFIER_LIST_h

#include "xcsMacros.h"

/* Condition of Classifier */
struct xCList{
  int pos;
  struct condAttribute value;
  struct xCList *next;
};

/* Condition of Classifier */
struct xCondition{
  int size;
  struct xCList *l;
};

int addXCPos(struct xCondition *con, int pos, char c);
int addXCBounds(struct xCondition *con, int pos, int low, int up);
int addXCRealBounds(struct xCondition *con, int pos, double low, double up);
void removeXCPos(struct xCondition *con, int pos);
struct xCondition *copyXC(struct xCondition *cOld);
void emptyXCondition(struct xCondition *con);
void resetXCSize(struct xCondition *con);
int isEqualXC(struct xCondition *c1, struct xCondition *c2);
void printXC(FILE *fp, struct xCondition *con);
char *getStringRepresentation(struct xCondition *con);
struct condAttribute *getAttributeArray(struct xCondition *con);
struct xCondition *getConditionCode(char *condition);
struct condAttribute *getAttribute(struct xCondition *con, int pos);

struct xClassifier{
  struct xCondition *con;
  int act;

  double pre;
  double preer;
  double acc;
  double fit;
  int num;
  int exp;
  double peerssest;
  int gaIterationTime;
};

struct xClassifierSet{
  struct xClassifier *cl;
  struct xClassifierSet *next;
};

struct XCS;

struct xClassifierSet * createRandomClassifierSet(int condLength, int maxPopSize, double dontCareProb, int doFitnessAdjustment, int doSetPreErrDirectly);
void createRandomCondition(struct xCondition *con, int condLength, double dontCareProb);
void createMatchingCondition(struct xCondition *con, struct attributes *state, struct XCS *xcs);
void createRandomAction(int *act);
void setInitialVariables(struct xClassifier *cl,int itTime, int doSetPreErrDirectly);

struct xClassifierSet *copyClassifierSet(struct xClassifierSet *pop);
struct xClassifierSet * getMatchSet(struct XCS *xcs, struct attributes *state, struct xClassifierSet **pop, struct xClassifierSet **killset, int itTime);
int getMatchSupport(struct attributes *state, struct xClassifierSet *c);
int getActionSupport(int action, struct xClassifierSet *c);
int matchBoolean(struct attributes *state, struct xCondition *c);
int match(struct attributes *state, struct xCondition *c);
int compareStateWithClassifier(struct attributes *state, struct xCondition *c);
/* Added on the 05/24/2000 to figure out how many actions are covered! */
int nrActionsInSet(struct xClassifierSet *set, int *coveredActions);
int getActionNr(char *action);
/* added on the 05/24/2000 to always cover all actions in one match set */
struct xClassifier * createNewCoverMatch(struct attributes *state, int itTime, struct xClassifierSet *set, int action, struct XCS *xcs);
/* added on the 05/24/2000 to always cover all actions in one match set */
int actionIsCovered(int action, struct xClassifierSet *set);

struct xClassifierSet * getActionSet(int action, struct xClassifierSet *ms);
void adjustActionSet(struct XCS *xcs, struct xClassifierSet **aset, double maxP, double reward, struct xClassifierSet **pop, struct xClassifierSet **killset);
void updateFitness(struct xClassifierSet *aset, struct XCS *xcs);
void discoveryComponent(struct xClassifierSet **set,struct xClassifierSet **pop,struct xClassifierSet **killset,int itTime, struct attributes *state, struct XCS *xcs, double reward);
int getNumerositySum(struct xClassifierSet *set);
void getDiscoversSums(struct xClassifierSet *set,double *sum,int *meangait,int *mssum,int *expSum, double *errSum);
void setTimeStamps(struct xClassifierSet *set, int itTime);
void selectTwoClassifiers(struct xClassifier **cl, struct xClassifier **parents, struct xClassifierSet *set, double fitsum, int setsum, struct XCS *xcs);
struct xClassifier * selectClassifierUsingTournamentSelection(struct xClassifierSet *set, int setsum, struct XCS *xcs, struct xClassifier *notMe);
struct xClassifier * selectClassifierUsingRWS(struct xClassifierSet *set,double sum);
void insertDiscoveredClassifier(struct xClassifier **cl, struct xClassifier **parents, struct xClassifierSet **set,
				struct xClassifierSet **pop, struct xClassifierSet **killset, int len, 
				struct XCS *xcs);

void doActionSetSubsumption(struct xClassifierSet **aset, struct xClassifierSet **pop, struct xClassifierSet **killset, int thetaSub, int doFitnessAdjustment);
void subsumeClassifier(struct xClassifier *cl,struct xClassifier **parents,struct xClassifierSet *locset,struct xClassifierSet **pop, int maxPopSize, int thetaSub, int doFitnessAdjustment);
int subsumeClassifierToSet(struct xClassifier *cl, struct xClassifierSet *set, int maxPopSize, int thetaSub, int doFitnessAdjustment);
int subsumes(struct xClassifier *cl1, struct xClassifier * cl2, int thetaSub);
int isSubsumer(struct xClassifier *cl, int thetaSub);
int isMoreGeneral (struct xCondition *first, struct xCondition *second);

int crossover(struct xClassifier **cl, int crossoverType, double chi);
int saveUniformCrossover(struct xClassifier **cl);
int uniformCrossover(struct xClassifier **cl);
int saveOnePointCrossover(struct xClassifier **cl);
int onePointCrossover(struct xClassifier **cl);
int twoPointCrossover(struct xClassifier **cl);
int informedCrossover(struct xClassifier **cl);

int saveMutation(struct xClassifier *cl, char *situation, struct XCS *xcs);
int mutation(struct xClassifier *cl, struct attributes *situation, struct XCS *xcs);
int mutate(char *c, char s, int doGeneralizationMutation, int doNicheMutation);
void mutateAction(int *action);

void generalize(struct xCondition *condition);
void specialize(struct xCondition *condition, char *situation);
double getSpecificity(struct xClassifier *cl);

int addClassifierToPointerSet(struct xClassifier *cl,struct xClassifierSet **pointerset);
int addClassifierToSet(struct xClassifier *cl,struct xClassifierSet **pop, int doFitnessAdjustment);
void addNewClassifierToSet(struct xClassifier *cl,struct xClassifierSet **pop);

struct xClassifier * deleteStochClassifier(struct xClassifierSet **pop, double delta, int thetaExp, int delType);
struct xClassifier * deleteTypeOfClassifier(struct xClassifierSet *setp,struct xClassifierSet *setpl,struct xClassifierSet **pop);
double getDelPropMikro(struct xClassifier *cl, double meanf, double meane, double delta, int thetaExp, int delType);
int updateSet(struct xClassifierSet **uset,struct xClassifierSet *killset);
int deleteClassifierPointerFromSet(struct xClassifierSet **set, struct xClassifier *clp);

void freeSet(struct xClassifierSet **head);
void freeClassifierSet(struct xClassifierSet **head);
void freeClassifier(struct xClassifier *cl);

void printClassifier(struct xClassifier*c);
void fprintClassifier(FILE *fp,struct xClassifier *c);
void printClassifierSet(struct xClassifierSet *head);	/*letzter Eintrag zu Testzwecken!!!*/
void fprintClassifierSet(FILE *fp,struct xClassifierSet *head);
void fprintMicroClassifierSet(FILE *fp,struct xClassifierSet *head);
struct xClassifierSet *readClassifierSet(FILE *fp);

struct xClassifierSet * sortClassifierSet(struct xClassifierSet **cls,int type);
struct xClassifierSet * filterClassifierSet(struct xClassifierSet *cls, double minPred, int minExp, int minNum, double maxError);
void getSpecAndSDev(struct xClassifierSet *set, double *spec, double *specSDev);
void getPosSpecificity(struct xClassifierSet *cll, double *specPos);

/* deletion stochastic with increase of deletion probability based on error
 * rather than on fitness
 */
struct xClassifier * deleteClassifier(struct xClassifierSet **pop, struct XCS *xcs);

void copyAttributes(struct attributes *to, struct attributes *from);
void fprintState(FILE *fp, struct attributes *state);
void fprintCondAttribute(FILE *fp, struct condAttribute att);
void fprintAttributes(FILE *fp, struct attributes att);

#endif
