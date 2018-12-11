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
/     Main program header
*/

#include "xcsMacros.h"
#include "classifierList.h"
#include "actionSelection.h"
//#include "env.h"
#include "treeRepresentation.h"

struct XCS{
  char *tabOutFile;
  int nrExps;
  int maxNrSteps;
  int initialSeed;
  int initialFoldSeed;
  int do10FoldedCrossValidation;
  int testFrequency;
  int popOutFrequency;
  int doGenerateTree;

  int maxPopSize;
  double alpha;

  double betaP;
  double betaE;
  double betaF;
  double betaS;

  double gamma;
  double epsilon0;
  int nu;
  double fallOffRate;

  int thetaGA;
  double fitnessReduction;
  int doInitializePE;
  int doSetPreErrDirectly; 

  int doFitnessAdjustment;
  double tournamentSize;
  double forceDifferentInTournament;
  double selectTolerance;
  int crossoverType;
  double chi;
  double mu;
  double mutateRand;
  double *mutRandAll;

  int doGeneralizationMutation;
  int doNicheMutation;
  
  int doMAM;
  int doGradient;
  int doRelativeFitness;

  int doGAErrorBasedSelect;
  
  double delta;
  int thetaDel;

  int deletionType;

  double dontCareProb;
  double coverRand;

  int doGASubsumption;
  int doActionSetSubsumption;
  int thetaSub;

  double exploreProb;
  int teletransportation;

  int initializePopulation; /* specifies if a random population should be generated initially */
};

struct xClassifierSet;

void startExperiments(struct XCS *xcs1);
struct XCS *readXCS(FILE *parameterFile);

void startOneSingleStepExperiment(struct XCS *xcs, struct xClassifierSet **pop, double **averages, int *expnr);
void startOneCrossValidationExperiment(struct XCS *xcs, struct xClassifierSet **pop, double **averages, int *expnr);
void doOneSingleStepProblemExplore(struct XCS *xcs, struct xClassifierSet **pop, struct attributes *state, int trialCounter);
void doOneSingleStepProblemExploit(struct XCS *xcs, struct xClassifierSet **pop, 
				   struct attributes *state, int trialCounter, 
				   int *correct, double *sysError, double *spec, double *specSDev);

void startOneMultiStepExperiment(struct XCS *xcs, struct xClassifierSet **pop, double **averages, int *expnr);
void doOneMultiStepProblemExplore(struct XCS *xcs, struct xClassifierSet **pop, struct attributes *state, int *counter);
void doOneMultiStepProblemExploit(struct XCS *xcs, struct xClassifierSet **pop,
				  struct attributes *state, int counter, int *stepToFood,
				  double *sysError, double *specSet, double *specSDevSet);
void writeFilePerformance(struct XCS *xcs, int testFrequency, struct xClassifierSet *pop, 
		      int *correct, double *sysError, double *specSet, double *specSDevSet, 
		      int counter, double **averages, int expnr, int multiStepSteps);
void writePerformance(struct XCS *xcs, int testFrequency, struct xClassifierSet *pop, 
		      int *correct, double *sysError, double *specSet, double *specSDevSet, 
		      int counter, double **averages, int expnr, int multiStepSteps, FILE *tabOutFile, FILE *popOutFile);
void writeAveragePerformance(struct XCS *xcs, double **averages);
double testClassifierSet(struct XCS *xcs, struct xClassifierSet *set);
void writeNicheSupport(struct XCS *xcs, struct xClassifierSet *pop, int expnr, int type);

void fprintAllStatistics(struct XCS *xcs, FILE *tabFile, double **allAverages, int counter, int expnr);
void freadStatistics(struct XCS *xcs, FILE *inFile, double **averages, int counter, int expnr);
int readInStatus(struct XCS *xcs, struct xClassifierSet **pop, double **averages, int *trialCounter, int *expnr);
struct XCS *initializeXCS(FILE *parameterFile);
void setMacroValues(struct XCS *xcs);
struct XCS *copyXCS(struct XCS *xcsOld);
void freeXCS(struct XCS *xcs);
void fprintXCS(FILE *outfile, struct XCS *xcs);
