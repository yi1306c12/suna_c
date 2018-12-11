/*
/       (XCS-C 1.2)
/	------------------------------------
/	Learning Classifier System based on accuracy
/
/     by Martin Butz
/     University of Wuerzburg / University of Illinois at Urbana/Champaign
/     butz@illigal.ge.uiuc.edu
/     Last modified: 08-28-2003
/
/     Main program
*/


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <unistd.h> 
#include <sys/resource.h> 

#include "xcs.h"


/*
int main(int args,char *argv[])
{
  FILE *env_file=NULL;
  FILE *parameterFile=NULL;
  struct XCS *xcsDefault;
  
  setpriority(PRIO_PROCESS, getpid(), 19);

  if(doNeedInputFile()){
    if(args<2){
      printf("Usage: xcs.out INPUTFILE [PARAMETERFILE]\n");
      return 0;
    }
    if ((env_file = fopen(argv[1], "rt")) == NULL){
      fprintf(stderr, "Cannot open file %s.\n",argv[1]);
      return 0;
    }
    if(!initEnv(env_file))
      return 0;
    fclose(env_file);
    
    if(args>2) {
      if( (parameterFile =fopen(argv[2], "rt")) == NULL){
	fprintf(stderr, "Cannot open parameter file %s.\n", argv[2]);
	return 0;
      }
    }
  }else{
    if(!initEnv(0))
      return 0;
    if(args>1) {
      if( (parameterFile =fopen(argv[1], "rt")) == NULL)  {
	fprintf(stderr, "Cannot open parameter file %s.\n", argv[1]);
	return 0;
      }
    }
  }

  xcsDefault = initializeXCS(parameterFile);
  if(parameterFile!=NULL) {
    fclose(parameterFile);
  }
  setSeed(xcsDefault->initialSeed);

  fprintXCS(stdout, xcsDefault);
  fprintEnv(stdout);
  
  startExperiments(xcsDefault);

  freeXCS(xcsDefault);
  freeEnv();

  return 1;
}
*/


void startExperiments(struct XCS *xcsDefault)
{
  int expcounter;
  struct xClassifierSet *pop, *help;
  struct XCS *xcsActual;
  double **averages, popAverage, filterAverage, treeAverage, treeFilterAverage;
  FILE *outFile, *treeFile;
  char *outFileName;
  struct Tree *tree, **forest;

  /* Make room to record all data to derive the standard deviation */
  assert((averages = (double **)(calloc( xcsDefault->nrExps , sizeof(double *))))!=0);
  for(expcounter=0; expcounter < xcsDefault->nrExps; expcounter++) {
    assert((averages[expcounter] = (double *)(calloc( (13 + getConditionLength()) * ((int)(xcsDefault->maxNrSteps/xcsDefault->testFrequency)+1), sizeof(double))))!=0);
  }

  /* start the experiments */
  for( expcounter=0 ; expcounter < xcsDefault->nrExps ; expcounter++ ) {
    xcsActual = copyXCS(xcsDefault);
    
    /* Initialize the population */
    pop=NULL;
    if(xcsActual->initializePopulation)
      pop=createRandomClassifierSet(getConditionLength(), xcsActual->maxPopSize, xcsActual->dontCareProb, xcsActual->doFitnessAdjustment, xcsActual->doSetPreErrDirectly);
    
    if(isMultiStep()) {
      startOneMultiStepExperiment(xcsActual, &pop, averages, &expcounter);
    }else{
      if(! xcsActual->do10FoldedCrossValidation) {
	startOneSingleStepExperiment(xcsActual, &pop, averages, &expcounter);
      }else{
	if(expcounter%10 == 0) {
	  /* prepares the environment for 10-folded cross-validation */
	  do10FoldedCrossValidation(xcsActual->initialFoldSeed + (int)(expcounter/10));
	  setSeed(xcsActual->initialSeed + (int)(expcounter/10));
	}
	startOneCrossValidationExperiment(xcsActual, &pop, averages, &expcounter);
      }
    }
    /* generate output file name incase population output is written in the end... */    
    /*
     * assert((outFileName = (char *)(calloc( strlen(xcsDefault->tabOutFile)+5, sizeof(char))))!=0);
     * strcpy(outFileName, xcsDefault->tabOutFile);
     * outFileName[strlen(xcsDefault->tabOutFile)]='.';
     * outFileName[strlen(xcsDefault->tabOutFile)+1]='t';
     * outFileName[strlen(xcsDefault->tabOutFile)+2]='x';
     * outFileName[strlen(xcsDefault->tabOutFile)+3]='t';
     * outFile = fopen(outFileName, "a+");
     * fprintf(outFile, "\nNum Sorted Classifier List:\n");
     * pop=sortClassifierSet(&pop, 0);
     * fprintClassifierSet(outFile,pop);
     * fclose(outFile);
     * free(outFileName);
     **/
    
    if(xcsDefault->doGenerateTree) {
      popAverage=treeAverage=filterAverage=treeFilterAverage = -1.;
      assert((outFileName = (char *)(calloc( strlen(xcsDefault->tabOutFile)+5, sizeof(char))))!=0);
      strcpy(outFileName, xcsDefault->tabOutFile);
      outFileName[strlen(xcsDefault->tabOutFile)]='.';
      outFileName[strlen(xcsDefault->tabOutFile)+1]='p';
      outFileName[strlen(xcsDefault->tabOutFile)+2]='t';
      outFileName[strlen(xcsDefault->tabOutFile)+3]='c';
      if(expcounter==0) {
	/* very first entry */
	if ((outFile = fopen(outFileName, "w")) == NULL) {
	  fprintf(stderr, "Cannot open file");
	  fprintf(stderr, outFileName);
	  return;
	}
	/* Write down parameter settings */
	fprintXCS(outFile, xcsActual);
	fprintEnv(outFile);
	fclose(outFile);
      }
      outFile = fopen(outFileName, "a+");
      
      outFileName[strlen(xcsDefault->tabOutFile)]='.';
      outFileName[strlen(xcsDefault->tabOutFile)+1]='t';
      outFileName[strlen(xcsDefault->tabOutFile)+2]='r';
      outFileName[strlen(xcsDefault->tabOutFile)+3]='e';
      treeFile = fopen(outFileName, "w+");
      /*intClassifierSet(*pop);*/
      
      /*popAverage = testClassifierSet(xcsActual, pop);*/
      
      help = filterClassifierSet(pop, 500, 20, 1, 400);
      
      if(help==NULL) {
	printf("Filtered Classifier List is empty!!!\n");
      }else{
	/*filterAverage = testClassifierSet(xcsActual, help);*/
	
	tree = generateTree(pop, xcsDefault);
	/*printTree(tree, 0, 0);*/
	/* fprintTree(treeFile, tree, 0);*/
	/*treeAverage = testTree(tree);*/
	deleteTree(&tree);
	
	tree = generateTree(help, xcsDefault);
	forest = generateForest(help, xcsDefault);

	/*printTree(tree, 0, 0);*/
	/*fprintDOTTree(treeFile, tree, 1);*/ 
	/*fprintAllTree(treeFile, tree, 1, xcsDefault);*/
	fprintAllForest(treeFile, forest, 1, xcsDefault);

	/*treeFilterAverage = testTree(tree);*/
	deleteTree(&tree);
	deleteForest(&forest);

	fprintf(outFile,"%f %f %f %f\n", popAverage, treeAverage, filterAverage, treeFilterAverage);
      }

      fclose(outFile);
      fclose(treeFile);
      freeSet(&help);
    }
    freeClassifierSet(&pop);
    freeXCS(xcsActual);
  }

  writeAveragePerformance(xcsDefault, averages);
  for(expcounter=0; expcounter < xcsDefault->nrExps; expcounter++) {
    free(averages[expcounter]);
  }
  free(averages);
}


/* ########################## in a single step environment ########################## */

void startOneSingleStepExperiment(struct XCS *xcs, struct xClassifierSet **pop, double **averages, int *expnr)
{
  int trialCounter, exploit=1;
  int *correct;
  double *sysError, *specSet, *specSDevSet;
  struct attributes *state;

  assert((correct=(int *)(calloc(xcs->testFrequency,sizeof(int))))!=0);
  assert((sysError=(double *)(calloc(xcs->testFrequency,sizeof(double))))!=0);
  assert((specSet=(double *)(calloc(xcs->testFrequency,sizeof(double))))!=0);
  assert((specSDevSet=(double *)(calloc(xcs->testFrequency,sizeof(double))))!=0);
  assert((state=(struct attributes *)(calloc(getConditionLength(),sizeof(struct attributes))))!=0);
  
  /* set the \0 char at the end of action and state */
  /*state[getConditionLength()]='\0';*/
  trialCounter = 0;

  /* check if a previous state is stored */
  if(*expnr==0) {
    if(readInStatus(xcs, pop, averages, &trialCounter, expnr)) {
   	printf("Successfully read status\n");
    }
  }

  /* Start one experiment, trialCounter counts the number of problems (trials)*/
  for(  ; trialCounter<xcs->maxNrSteps ; trialCounter+=exploit) {
    /* change from explore to exploit and backwards */
    exploit= (exploit+1)%2;

    resetState(state);

    if(!exploit)
      doOneSingleStepProblemExplore(xcs, pop, state, trialCounter);
    else
      doOneSingleStepProblemExploit(xcs, pop, state, trialCounter, &correct[trialCounter%(xcs->testFrequency)], &sysError[trialCounter%(xcs->testFrequency)], &specSet[trialCounter%(xcs->testFrequency)], &specSDevSet[trialCounter%(xcs->testFrequency)]);
    
    if( trialCounter%xcs->testFrequency==0 && exploit && trialCounter>0 ) {
      writeFilePerformance(xcs, xcs->testFrequency, *pop, correct, sysError, specSet, specSDevSet, trialCounter, averages, *expnr, 0);
    }
  }
  if( trialCounter%xcs->testFrequency==0 && exploit && trialCounter>0 ) {
    writeFilePerformance(xcs, xcs->testFrequency, *pop, correct, sysError, specSet, specSDevSet, trialCounter, averages, *expnr, 0);
  }
  writeNicheSupport(xcs, *pop, *expnr, 3);
  
  free(correct);
  free(sysError);
  free(specSet);
  free(specSDevSet);
  free(state);
}


void startOneCrossValidationExperiment(struct XCS *xcs, struct xClassifierSet **pop, double **averages, int *expnr)
{
  int trialCounter, maxSteps, segmentSize;
  int *correct;
  double *sysError, *specSet, *specSDevSet;
  struct attributes *state;

  /* set the \0 char at the end of action and state */
  /*state[getConditionLength()]='\0';*/
  trialCounter = 0;

  /* check if a previous state is stored */
  if(*expnr==0) {
    if(readInStatus(xcs, pop, averages, &trialCounter, expnr))
   	printf("Successfully read status\n");
  }
  
  printf("Cross-validation learning...\n");
  assert((correct=(int *)(calloc(xcs->testFrequency,sizeof(int))))!=0);
  assert((sysError=(double *)(calloc(xcs->testFrequency,sizeof(double))))!=0);
  assert((specSet=(double *)(calloc(xcs->testFrequency,sizeof(double))))!=0);
  assert((specSDevSet=(double *)(calloc(xcs->testFrequency,sizeof(double))))!=0);
  assert((state=(struct attributes *)(calloc(getConditionLength(),sizeof(struct attributes))))!=0);
  /* Start one experiment, trialCounter counts the number of problems (trials)*/
  for(  ; trialCounter<xcs->maxNrSteps ; trialCounter++) {
    if(trialCounter>0 && trialCounter%(xcs->testFrequency)==0) {
      ;/*printf("Done %d steps\n", trialCounter);*/
    }
    doOneSingleStepProblemExplore(xcs, pop, state, trialCounter);
    resetState(state);
    doOneSingleStepProblemExploit(xcs, pop, state, trialCounter, &correct[trialCounter%(xcs->testFrequency)], &sysError[trialCounter%(xcs->testFrequency)], &specSet[trialCounter%(xcs->testFrequency)], &specSDevSet[trialCounter%(xcs->testFrequency)]);
    if( trialCounter%xcs->testFrequency==0 && trialCounter>0 ) {
      ;/*writePerformance(xcs, xcs->testFrequency, *pop, correct, sysError, specSet, specSDevSet, trialCounter, averages, *expnr, 0, stdout, NULL);*/
    }
  }
  free(correct);
  free(sysError);
  free(specSet);
  free(specSDevSet);

  /* Now test the learned representation */
  /*printf("Testing learned solution...\n");*/
  maxSteps = doTesting(1);
  if(maxSteps>2000000) 
    segmentSize=1000000;
  else
    segmentSize=maxSteps;
  assert((correct=(int *)(calloc(segmentSize,sizeof(int))))!=0);
  assert((sysError=(double *)(calloc(segmentSize,sizeof(double))))!=0);
  assert((specSet=(double *)(calloc(segmentSize,sizeof(double))))!=0);
  assert((specSDevSet=(double *)(calloc(segmentSize,sizeof(double))))!=0);  
  for(trialCounter=0; trialCounter < maxSteps; trialCounter++) {
    nextTestState(state);
    
    doOneSingleStepProblemExploit(xcs, pop, state, xcs->maxNrSteps+trialCounter, &correct[trialCounter%segmentSize], &sysError[trialCounter%segmentSize], &specSet[trialCounter%segmentSize], &specSDevSet[trialCounter%segmentSize]);
    if( (trialCounter+1) % segmentSize == 0 && (trialCounter+1) != maxSteps) {
      /*printf("Writing chunk performance \n");*/
      writeFilePerformance(xcs, segmentSize, *pop, correct, sysError, specSet, specSDevSet, trialCounter, averages, *expnr, 0);
    }
  }
  if(maxSteps>segmentSize) {
    /* size of final segment */
    segmentSize = maxSteps % segmentSize;
  }
  /*printf("Writing final performance of length %d\n", segmentSize);*/
  writeFilePerformance(xcs, segmentSize, *pop, correct, sysError, specSet, specSDevSet, trialCounter, averages, *expnr, 0);
  free(correct);
  free(sysError);
  free(specSet);
  free(specSDevSet);
  doTesting(0);
  /*printf("Done testing.\n");*/

  /*writeNicheSupport(xcs, *pop, *expnr, 3);*/
  
  free(state);
}

/* one explore step in a single step problem */
void doOneSingleStepProblemExplore(struct XCS *xcs, struct xClassifierSet **pop, struct attributes *state, int trialCounter)
{        
  struct xClassifierSet *mset, *aset, *killset=NULL;
  int action;
  double reward=0., *predictionArray;
  int correct;

  assert((predictionArray = (double *)calloc(getNumberOfActions(),sizeof(double)))!=0);
  
  /* get the match set */
  mset=getMatchSet(xcs, state, pop, &killset, trialCounter);

  /* no updates are necessary in this case */
  freeSet(&killset);

  /* get the Prediction array */
  getPredictionArray(mset, predictionArray, xcs);
    
  /* Get the action, that wins in the prediction array */
  action = learningActionWinner( predictionArray, xcs->exploreProb);
  
  /* execute the action and get reward
   * correct represents a boolean for the right or wrong action */
  reward = doAction(state, action, &correct);
  
  /* Get the action set according to the chosen action aw */
  aset = getActionSet(action, mset);
  
  /* Give immediate reward */
  adjustActionSet(xcs, &aset,0,reward, pop, &killset);
  /* no update necessary here */
  freeSet(&killset);  
  
  /* Exectue the discovery mechanism */
  discoveryComponent(&aset,pop,&killset,trialCounter, state, xcs, reward);
  
  /* no update necessary here */
  freeSet(&killset);
  
  /* Clean up */
  freeSet(&mset);
  freeSet(&aset);
  free(predictionArray);
}

/* one test step */
void doOneSingleStepProblemExploit(struct XCS *xcs, struct xClassifierSet **pop, 
				   struct attributes *state, int trialCounter, 
				   int *correct, double *sysError, double *spec, double *specSDev)
{        
  struct xClassifierSet *mset, *killset=NULL;
  int action;
  double reward=0., *predictionArray;
  
  assert((predictionArray = (double *)calloc(getNumberOfActions(),sizeof(double)))!=0);
  
  /* get the match set*/
  mset=getMatchSet(xcs, state, pop, &killset, trialCounter);
  
  getSpecAndSDev(mset, spec, specSDev);

  /* no updates are necessary in this case */
  freeSet(&killset);
  
  /* get the Prediction array */
  getPredictionArray(mset, predictionArray, xcs);
  
  /* Get the action, that wins in the prediction array */
  action = deterministicActionWinner(predictionArray);
  
  /* execute the action and get reward
   * correct represents a boolean for the right or wrong action */
  reward = doAction(state, action, correct);
  
  /* remember the system error */
  *sysError=(double)(abs((int)(reward - predictionArray[action])))/getPaymentRange();
  
  /* Clean up */
  freeSet(&mset);
  free(predictionArray);
}





/* ########################## in a multi step environment ########################## */

void startOneMultiStepExperiment(struct XCS *xcs, struct xClassifierSet **pop, double **averages, int *expnr)
{
  int counter, trialCounter, exploit=0;
  int *stepToFood;
  double *sysError, *specSet, *specSDevSet;
  struct attributes *state;

  assert((stepToFood=(int *)(calloc(xcs->testFrequency,sizeof(int))))!=0);
  assert((sysError=(double *)(calloc(xcs->testFrequency,sizeof(double))))!=0);
  assert((specSet=(double *)(calloc(xcs->testFrequency,sizeof(double))))!=0);
  assert((specSDevSet=(double *)(calloc(xcs->testFrequency,sizeof(double))))!=0);
  assert((state=(struct attributes *)(calloc(getConditionLength(),sizeof(struct attributes))))!=0);

  /* set the \0 char at the end of action and state */
  /*state[getConditionLength()]='\0';*/
  
  trialCounter=0;
     
  /* check if a previous state is stored */
  if(*expnr==0) {
    if(readInStatus(xcs, pop, averages, &trialCounter, expnr))
   	printf("Successfully read status\n");
  }

  /* Start one experiment, trialCounter counts the number of exploit problems (trials)*/
  for( counter=0 ; trialCounter<=xcs->maxNrSteps ; trialCounter+=exploit) {
    
    exploit= (exploit+1)%2;
    
    if(!exploit)
      doOneMultiStepProblemExplore(xcs, pop, state, &counter);
    else
      doOneMultiStepProblemExploit(xcs, pop, state, counter, &stepToFood[trialCounter%(xcs->testFrequency)], &sysError[trialCounter%(xcs->testFrequency)], &specSet[trialCounter%(xcs->testFrequency)], &specSDevSet[trialCounter%(xcs->testFrequency)]);
    
    /* write out the performance every xcs->testFrequency trials */
    if( trialCounter%(xcs->testFrequency)==0 && exploit && trialCounter>0 ){
      writeFilePerformance(xcs, xcs->testFrequency, *pop, stepToFood, sysError, specSet, specSDevSet, trialCounter, averages, *expnr, counter);
    }
  }
  free(stepToFood);
  free(sysError);
  free(specSet);
  free(specSDevSet);
  free(state);
}

void doOneMultiStepProblemExplore(struct XCS *xcs, struct xClassifierSet **pop, struct attributes *state, int *counter)
{
  double reward=0., previousReward=0., *predictionArray, maxPredArray;
  struct attributes *lastState, *previousState;
  struct xClassifierSet *mset, *aset, *paset=NULL, *killset=NULL;
  int action, stepCounter, reset=0;

  assert((predictionArray = (double *)calloc(getNumberOfActions(),sizeof(double)))!=0);

  assert((lastState=(struct attributes *)calloc(getConditionLength(),sizeof(struct attributes)))!=NULL);
  assert((previousState=(struct attributes *)calloc(getConditionLength(),sizeof(struct attributes)))!=NULL);

  resetState(state);

  /* Start one problem, stepCounter counts the number of steps executed */
  for( stepCounter=0 ; stepCounter<xcs->teletransportation && !reset; stepCounter++, (*counter)++) {

    /* get the match set and update the previous action set*/
    mset=getMatchSet(xcs, state,pop,&killset,(*counter));
    if( paset!=NULL)
      updateSet(&paset,killset);
    freeSet(&killset);

    /* get the Prediction array */
    getPredictionArray(mset, predictionArray, xcs);

    /* Get the action, that wins in the prediction array */
    action = learningActionWinner(predictionArray, xcs->exploreProb);

    /* Get the action set according to the chosen action aw */
    aset = getActionSet(action, mset);

    /* execute the action and get reward */
    copyAttributes(lastState, state);

    reward = doAction(state, action, &reset);

    /* Backpropagate the reward to the previous action set and apply the GA */
    if( paset!=NULL) {
      maxPredArray = predictionArray[deterministicActionWinner(predictionArray)];
      adjustActionSet(xcs, &paset, maxPredArray, previousReward, pop, &killset);
      updateSet(&aset,killset);
      freeSet(&killset);
      discoveryComponent(&paset,pop,&killset,(*counter),previousState, xcs, previousReward*xcs->gamma*maxPredArray);
      updateSet(&aset,killset);
      freeSet(&killset);
    }

    /* Give immediate reward, if a reset will take place and apply the GA, too */
    if( reset ){
      adjustActionSet(xcs, &aset, 0, reward, pop, &killset);
      freeSet(&killset);
      discoveryComponent(&aset,pop,&killset,(*counter),lastState, xcs, reward);
      freeSet(&killset);
    }
    /* Clean up */
    freeSet(&mset);
    freeSet(&paset);
    previousReward=reward;
    paset=aset;
    copyAttributes(previousState, lastState);
  }
  freeSet(&paset);
  free(previousState);
  free(lastState);
  free(predictionArray);
}

void doOneMultiStepProblemExploit(struct XCS *xcs, struct xClassifierSet **pop,
				  struct attributes *state, int counter, int *stepToFood,
				  double *sysError, double *specSet, double *specSDevSet)
{
  double reward=0., previousReward=0., *predictionArray, predictionValue, previousPrediction=0.;
  double specS=0, specSDevS=0;
  struct xClassifierSet *mset, *aset, *paset=NULL, *killset=NULL;
  int action, stepCounter, reset=0;

  assert((predictionArray = (double *)calloc(getNumberOfActions(),sizeof(double)))!=0);

  /* set the \0 char at the end of action and init the sysError*/
  *sysError=0;
  *specSet=0;
  *specSDevSet=0;
  resetState(state);

  /* Start one problem, stepCounter counts the number of steps executed */
  for( stepCounter=0 ; stepCounter<xcs->teletransportation && !reset ; stepCounter++) {

    /* get the match set and update the previous action set*/
    mset=getMatchSet(xcs, state, pop, &killset, counter);
    if( paset!=NULL)
      updateSet(&paset,killset);
    freeSet(&killset);

    /* get the Prediction array */
    getPredictionArray(mset, predictionArray, xcs);

    /* Get the action, that wins in the prediction array */
    action = deterministicActionWinner(predictionArray);
    predictionValue= predictionArray[action];

    /* Get the action set according to the chosen action aw */
    aset = getActionSet(action, mset);

    /* execute the action and get reward */
    reward = doAction(state, action, &reset);

    /* Backpropagate the reward to the previous action set */
    if( paset!=NULL) {
      adjustActionSet(xcs, &paset, predictionValue, previousReward, pop, &killset);
      updateSet(&aset, killset);
      freeSet(&killset);
      (*sysError) += (double)(abs((int)(previousReward + (xcs->gamma) *predictionValue - previousPrediction))) / (double)getPaymentRange();
      getSpecAndSDev(paset, &specS, &specSDevS);
      (*specSet) += specS;
      (*specSDevSet) += specSDevS;
    }

    /* Give immediate reward, if a reset will take place */
    if( reset ){
      adjustActionSet(xcs, &aset, 0, reward, pop, &killset);
      freeSet(&killset);
      (*sysError) += (double)(abs((int)(reward - predictionValue)))/getPaymentRange();
      getSpecAndSDev(aset, &specS, &specSDevS);
      (*specSet) += specS;
      (*specSDevSet) += specSDevS;
    }

    /* remind the prediction for the system Error */
    previousPrediction=predictionValue;
    previousReward=reward;
    /* Clean up */
    freeSet(&mset);
    freeSet(&paset);
    paset=aset;
  }
  freeSet(&paset);
  *stepToFood=stepCounter;
  (*sysError) /= stepCounter;
  (*specSet) /= stepCounter;
  (*specSDevSet) /= stepCounter;
  free(predictionArray);
}


/**
 * Tests the given classifier set on the test set provided by the environment.
 * Returns fraction correct.
 */
double testClassifierSet(struct XCS *xcs, struct xClassifierSet *set)
{
  struct attributes *state;
  int correct, correctSum, tests;
  double error, spec, specSDev;
  
  /*printf("Testing Classifier Set...\n");*/
  assert((state=(struct attributes *)(calloc(getConditionLength(),sizeof(struct attributes))))!=0);
  correct=0;
  correctSum=0;
  
  doTesting(1);
  tests=0;
  while(nextTestState(state)) {
    tests++;      
    doOneSingleStepProblemExploit(xcs, &set, state, xcs->maxNrSteps+tests, &correct, &error, &spec, &specSDev);
    correctSum +=correct;
  }
  free(state);
  /*printf(" Classifier Set Classified %d out of %d correct!\n", correctSum, tests);*/
  return (double)correctSum/(double)tests;  
}

/**
 * Requests niches from the population and writes the corresponding niche support
 * "type" specifies if whole classifier should be printed (%2=0) or only condition parts (%2=1)
 * type also specifies if niches are represented by a sample (type>1) or by a schema (type<2)
 */
void writeNicheSupport(struct XCS *xcs, struct xClassifierSet *pop, int expnr, int type)
{
  struct xClassifierSet *p;
  struct attributes *state;
  char *outFileName;
  int i, sum;
  FILE *tabFile;
  
  assert((outFileName = (char *)(calloc( strlen(xcs->tabOutFile)+5, sizeof(char))))!=0);

  strcpy(outFileName, xcs->tabOutFile);
  outFileName[strlen(xcs->tabOutFile)]='.';
  outFileName[strlen(xcs->tabOutFile)+1]='n';
  outFileName[strlen(xcs->tabOutFile)+2]='i';
  outFileName[strlen(xcs->tabOutFile)+3]='c';

  if(expnr==0) {
    /* very first entry */
    if ((tabFile = fopen(outFileName, "w")) == NULL) {
      fprintf(stderr, "Cannot open file");
      fprintf(stderr, outFileName);
      return;
    }
    /* Write down parameter settings */
    fprintXCS(tabFile, xcs);
    fprintEnv(tabFile);
    fclose(tabFile);
  }
  
  /* Open files for statistics*/
  if ((tabFile = fopen(outFileName, "a+")) == NULL) {
    fprintf(stderr, "Cannot open file");
    fprintf(stderr, outFileName);
    return;
  }

  assert((state=(struct attributes *)(calloc(getConditionLength(),sizeof(struct attributes))))!=0);
  /*state[getConditionLength()]='\0';*/
  
  resetState(state);
  while(getNextNiche(state, (int)(type/2))) {
    /*for(action=0; action<getNumberOfActions(); action++) {*/
    /*sum=0;
     * for(p=pop; p!=NULL; p=p->next) {
     *	if(p->cl->act == action && compareStateWithClassifier(state, p->cl->con)%2==0) { 
     */
    /* classifier is equal or more specific */
    /*printf("%s-%d: ",state,action);
     *printClassifier(p->cl);
     *printf("\n");*/
    /*sum += p->cl->num;
     *}
     *}*/
    /*printf("%s: %d\n", state, sum);*/
    
    /*fprintf(tabFile, "%d ", sum);*/
    /*if(sum>0) {*/
    /*fprintf(tabFile, "\n# ");*/
    fprintState(tabFile, state);
    /*fprintf(tabFile, ": %d\n", sum);*/
    fprintf(tabFile, "\n");
    for(p=pop; p!=NULL; p=p->next) {
      if(/*p->cl->act == action && */match(state, p->cl->con)) { /* classifier matches */
	if(type%2 == 0) {
	  fprintClassifier(tabFile, p->cl);
	  fprintf(tabFile, "\n");
	}else{
	  for(i=0; i<p->cl->num; i++) {
	    printXC(tabFile, p->cl->con);
	    fprintf(tabFile, "\n");
	  }
	}
	sum += p->cl->num;

      }
    }
    fprintf(tabFile, "\n");
    /*
      }*/
    /*    }*/
  }
  /*fprintf(tabFile, " *\n");*/

  fclose(tabFile);
  free(outFileName);
}

void writeFilePerformance(struct XCS *xcs, int testFrequency, struct xClassifierSet *pop, 
		      int *correct, double *sysError, double *specSet, double *specSDevSet, 
		      int counter, double **averages, int expnr, int multiStepSteps)
{
  FILE *tabFile, *popOutFile;
  char *outFileName;
  
  assert((outFileName = (char *)(calloc( strlen(xcs->tabOutFile)+5, sizeof(char))))!=0);
  
  strcpy(outFileName, xcs->tabOutFile);
  outFileName[strlen(xcs->tabOutFile)]='.';
  outFileName[strlen(xcs->tabOutFile)+1]='t';
  outFileName[strlen(xcs->tabOutFile)+2]='x';
  outFileName[strlen(xcs->tabOutFile)+3]='t';

  if(expnr==0 && counter==testFrequency) {
    /* very first entry */
    if ((tabFile = fopen(outFileName, "w")) == NULL) {
      fprintf(stderr, "Cannot open file");
      fprintf(stderr, outFileName);
      return;
    }
    /* Write down parameter settings */
    fprintXCS(tabFile, xcs);
    fprintEnv(tabFile);
    fclose(tabFile);
  }
  
  /* Open files for statistics*/
  if ((tabFile = fopen(outFileName, "a+")) == NULL) {
    fprintf(stderr, "Cannot open file");
    fprintf(stderr, outFileName);
    return;
  }

  popOutFile = NULL;
  if(counter % xcs->popOutFrequency == 0) {
    outFileName[strlen(xcs->tabOutFile)]='.';
    outFileName[strlen(xcs->tabOutFile)+1]='p';
    outFileName[strlen(xcs->tabOutFile)+2]='o';
    outFileName[strlen(xcs->tabOutFile)+3]='p';
    if((popOutFile=fopen(outFileName, "wt"))==0)
      return;
  }
  free(outFileName);

  writePerformance(xcs, testFrequency, pop, correct, sysError, specSet, specSDevSet, counter, averages, expnr, multiStepSteps, tabFile, popOutFile);

  fclose(tabFile);
  if(popOutFile!=NULL) 
    fclose(popOutFile);
}

/* writes the performance averaged over the last testFrequency trials */
void writePerformance(struct XCS *xcs, int testFrequency, struct xClassifierSet *pop, 
		      int *correct, double *sysError, double *specSet, double *specSDevSet, 
		      int counter, double **averages, int expnr, int multiStepSteps, 
		      FILE *tabFile, FILE *popOutFile)
{
  double corr=0.,serr=0., specPop=0., specSDevPop=0., assest=0., specS=0, specSDevS=0;
  double perr=0., perrdev=0., fit=0., exp=0., *specPos=0;
  int i, popsize=0, popnum=0, pos=0;
  struct xClassifierSet *popp;

  if(counter==testFrequency) {
    fprintf(tabFile, "# Next Experiment\n");
  }
  assert((specPos=(double *)calloc(getConditionLength(), sizeof(double)))!=NULL);
  getPosSpecificity(pop, specPos);

  for( popp=pop, popsize=0, popnum=0 ; popp!=NULL ;  popp=popp->next ) {
    popnum += popp->cl->num; /* Just count the size of the population */
    popsize++;
    assest += popp->cl->peerssest*popp->cl->num;
    perr += popp->cl->preer*popp->cl->num;
    fit += popp->cl->fit;
    exp += (double)popp->cl->exp * popp->cl->num;
  }
  assest /= (double)popnum;
  perr /= (double)popnum;
  fit /= (double)popnum;
  exp /= (double)popnum;

  /* determination of standard deviation of prediction error */
  for( popp=pop ; popp!=NULL ;  popp=popp->next ) {
    perrdev += (popp->cl->preer-perr)*(popp->cl->preer-perr)*popp->cl->num;
  }
  perrdev = sqrt(perrdev/(double)popnum);

  getSpecAndSDev(pop, &specPop, &specSDevPop);
  
  for(i=0;i<testFrequency;i++){
    corr+=correct[i];
    serr+=sysError[i];
    specS+=specSet[i];
    specSDevS+=specSDevSet[i];
  }
  corr/=(testFrequency);
  serr/=(testFrequency);
  specS/=(testFrequency);
  specSDevS/=(testFrequency);
  if(counter%1000==0){
    /*printf("t=%d %f %f |[P]|=%d assest=%f exp=%f specPop=%f specSDevPop=%f, perr=%f perrdev=%f fit=%f specS=%f specSDevS=%f",counter, corr, serr, popsize, assest, exp, specPop, specSDevPop, perr, perrdev, fit, specS, specSDevS);*/
   if(isMultiStep())
     ;/*printf("time=%d ", multiStepSteps);*/
   for(i=0; i<getConditionLength(); i++){
     ;/*printf(" %f",((double)specPos[i]/popnum));*/
   }
   /*printf("\n");*/

  }
  /* fprintf(tabFile, "%d %f %f %d",counter, corr, serr, popsize);*/
  fprintf(tabFile, "%d %f %f %d %f %f %f %f %f %f %f %f %f", counter, corr, serr, popsize, assest, exp, specPop, specSDevPop, perr, perrdev, fit, specS, specSDevS);
  if(isMultiStep())
    fprintf(tabFile, " %d", multiStepSteps);

  pos = (13+getConditionLength()) * (int)((counter/(testFrequency))-1);
  averages[expnr][pos] = corr;
  averages[expnr][pos+1] = serr;
  averages[expnr][pos+2] = popsize;
  averages[expnr][pos+3] = assest;
  averages[expnr][pos+4] = exp;
  averages[expnr][pos+5] = specPop;
  averages[expnr][pos+6] = specSDevPop;
  averages[expnr][pos+7] = perr;
  averages[expnr][pos+8] = perrdev;
  averages[expnr][pos+9] = fit;
  averages[expnr][pos+10] = specS;
  averages[expnr][pos+11] = specSDevS;
  averages[expnr][pos+12] = multiStepSteps;
  for(i=0; i<getConditionLength(); i++) {
    averages[expnr][pos+13+i] = ((double)specPos[i]/popnum);
  }

  fprintf(tabFile, "\t");
  for(i=0; i<getConditionLength(); i++) {
    fprintf(tabFile, " %f",((double)specPos[i]/popnum));
  }
  fprintf(tabFile, "\n");

  /* possibly write out current population status */
  if(popOutFile != NULL) {
    fprintf(popOutFile, "%d\n", counter);
    fprintf(popOutFile, "%d\n", expnr);
    /* fprintStatistics(xcs, popOutFile, averages[expnr], counter);*/
    fprintClassifierSet(popOutFile, pop);
  }
  free(specPos);
}

/* Writes out averages and the corresponding standard deviations */
void writeAveragePerformance(struct XCS *xcs, double **averages)
{
  char *tabFileName;
  int i,pos,j, exp;
  FILE *tabFile;
  double *av, *stdev;

  assert((av = (double *)(calloc( (13 + getConditionLength()) * 
      ((int)(xcs->maxNrSteps/xcs->testFrequency)+1), sizeof(double))))!=0);
  assert((stdev = (double *)(calloc( (13 + getConditionLength()) * 
      ((int)(xcs->maxNrSteps/xcs->testFrequency)+1), sizeof(double))))!=0);
  assert((tabFileName = (char *)(calloc( strlen(xcs->tabOutFile)+5, sizeof(char))))!=0);

  strcpy(tabFileName,xcs->tabOutFile);
  tabFileName[strlen(xcs->tabOutFile)]='.';
  tabFileName[strlen(xcs->tabOutFile)+1]='t';
  tabFileName[strlen(xcs->tabOutFile)+2]='a';
  tabFileName[strlen(xcs->tabOutFile)+3]='b';
  /* Open files for statistics*/
  if ((tabFile = fopen(tabFileName, "wt")) == NULL) {
    fprintf(stderr, "Cannot open file");
    fprintf(stderr, tabFileName);
    return;
  }
  free(tabFileName);

  fprintf(tabFile, "# The following averages were determined\n");
  fprintXCS(tabFile, xcs);
  fprintEnv(tabFile);

  for(exp=0; exp < xcs->nrExps; exp++) {
    for(i=0; i < (13+getConditionLength())*((int)(xcs->maxNrSteps/xcs->testFrequency)+1); i++) {
      av[i] += averages[exp][i];
    }
  }
  for(i=0; i < (13+getConditionLength())*((int)(xcs->maxNrSteps/xcs->testFrequency)+1); i++) {
    av[i] /= xcs->nrExps;
  }
  for(exp=0; exp < xcs->nrExps; exp++) {
    for(i=0; i < (13+getConditionLength())*((int)(xcs->maxNrSteps/xcs->testFrequency)+1); i++) {
      stdev[i] += (av[i]-averages[exp][i])*(av[i]-averages[exp][i]);
    }
  }
  for(i=0; i < (13+getConditionLength())*((int)(xcs->maxNrSteps/xcs->testFrequency)+1); i++) {
    stdev[i] = sqrt(stdev[i]/xcs->nrExps);
  }

  for(i=0; i<(int)(xcs->maxNrSteps/xcs->testFrequency); i++) {
    fprintf(tabFile, "%d ", (1+i)*xcs->testFrequency);
    pos = i*(13+getConditionLength());
    for(j=0; j<13+getConditionLength(); j++) {
      if(j==2) {/*this is the population size */
	fprintf(tabFile, "%f %f ", av[pos+j]/xcs->maxPopSize, stdev[pos+j]/xcs->maxPopSize);
      }else{
	fprintf(tabFile, "%f %f ", av[pos+j], stdev[pos+j]);
      }
    }
    fprintf(tabFile, "\n");
  }
  fclose(tabFile);
  free(av);
  free(stdev);
}


void fprintAllStatistics(struct XCS *xcs, FILE *tabFile, double **allAverages, int counter, int expnr)
{
  int i, j, k, pos;
  double *averages;
  
  fprintXCS(tabFile, xcs);
  fprintEnv(tabFile);
  
  pos = (13+getConditionLength());
  
  for(i=0; i<=expnr; i++) {
    fprintf(tabFile, "# Next Experiment\n");
    averages=allAverages[i];
    for(j=0; j < (xcs->maxNrSteps)/(xcs->testFrequency) && (i<expnr || j < counter/(xcs->testFrequency)); j++) {
      fprintf(tabFile, "%d ", (j+1)*xcs->testFrequency);
      for(k=0; k < 13+getConditionLength(); k++) {
	fprintf(tabFile, "%f ", averages[j*pos+k]);
      }
      fprintf(tabFile, "\n");
    }
  }
}

void freadStatistics(struct XCS *xcs, FILE *inFile, double **allAverages, int counter, int expnr)
{
  int i,j,k,l,pos,exp;
  char s[5000], value[500];
  double *averages;

  pos = (13+getConditionLength());

  for(exp=0, i=0; exp<(expnr+1) || i < counter/(xcs->testFrequency); i++) {
    do{
      if(strcmp(s,"# Next Experiment")==0) {
	/*printf("Next exp...###################################################\n");*/
	i=0;
	exp++;
      }
      assert(fscanf(inFile,"%[^\n]\n",s)!=0);
    }while(s[0]=='#');
    if(exp==0) {
      printf("Error in reading stats!\n");
      return;
    }
    averages=allAverages[exp-1];
    l=0;
    for(j=-1; j<13+getConditionLength(); j++) {      
      if(j==12 && !isMultiStep()) { /* single step does not record extra time entry */
	averages[i*pos+j]=0.;
	j++;
      }
      while(s[l]==' ' || s[l]=='\0' || s[l]=='\t') {
	l++;
      }
      k=0;
      while(s[l]!=' ' && s[l]!='\0' && s[l]!='\t') {
	value[k]=s[l];
	k++;
	l++;
      }
      value[k]='\0'; /* first value is the step counter */
      if(j>=0)
	averages[i*pos+j]=getDoubleValue(value);
    }
  }
}

/**
 * Tries to read in previous run that ended prematurely, if available...
 */
int readInStatus(struct XCS *xcs, struct xClassifierSet **pop, double **averages, int *trialCounter, int *expnr)
{
  char s[500], *fileName;
  FILE *popFile, *statFile;

  assert((fileName = (char *)(calloc( strlen(xcs->tabOutFile)+5, sizeof(char))))!=0);
  /* check if previous state was saved... */
  strcpy(fileName, xcs->tabOutFile);
  fileName[strlen(xcs->tabOutFile)]='.';
  fileName[strlen(xcs->tabOutFile)+1]='p';
  fileName[strlen(xcs->tabOutFile)+2]='o';
  fileName[strlen(xcs->tabOutFile)+3]='p';  
  /* Open files for population*/
  if ((popFile = fopen(fileName, "rt")) != NULL) {
    printf("Opened popfile %s\n", fileName);
    freeClassifierSet(pop);
  }else{
    printf("Starting from the beginning!\n");
    return 0;
  }
  /* read trial counter */
  if(fscanf(popFile,"%[^\n]\n",s)!=1)
    return 0;
  *trialCounter = getIntValue(s);
  /* read experiment number */
  if(fscanf(popFile,"%[^\n]\n",s)!=1)
    return 0;
  *expnr = getIntValue(s);
  /* read in current populatoion */
  *pop = readClassifierSet(popFile);
  /* close the file */
  fclose(popFile);

  /* finally, read previous statistics */
  fileName[strlen(xcs->tabOutFile)+1]='t';
  fileName[strlen(xcs->tabOutFile)+2]='x';
  fileName[strlen(xcs->tabOutFile)+3]='t';
  /* Open files for statistics*/
  if ((statFile = fopen(fileName, "r")) != NULL) {
    printf("Opened stat file %s\n", fileName);
  }else{
    printf("Could not open/find stat file %s\n", fileName);
    return 0;
  }
  freadStatistics(xcs, statFile, averages, *trialCounter, *expnr);
  fclose(statFile);
  /* rewrite all statistics until this point */
  if ((statFile = fopen(fileName, "w")) != NULL) {
    printf("Rewriting stat file %s\n", fileName);
  }
  fprintAllStatistics(xcs, statFile, averages, *trialCounter, *expnr);
  fclose(statFile);
  free(fileName);
  /* go to next problem instance */
  (*trialCounter)++;
  return 1;
}


/* initialize XCS parameter struct */
struct XCS *initializeXCS(FILE *parameterFile)
{
  char s[500], type[100], value[100];
  int i, j, xs, cLength;
  struct XCS *xcs=NULL;
  
  assert((xcs=( struct XCS *)calloc(1,sizeof(struct XCS)))!=NULL);

  cLength = getConditionLength();
  setMacroValues(xcs);

  if(parameterFile){
    while(fscanf(parameterFile,"%[^\n]\n",s)==1){
      if(s[0]!='#' && s[0]!='\n'){  /* '#' is the comment sign, empty lines are not treated */ 
	xs=strlen(s);
	for(i=0; i<xs && s[i]!=' ' && s[i]!='\t' && i<100; i++)
	  type[i]=s[i];
	if(i==100){
	  printf("Error in parameter File - input too long!");
	  return NULL;
	}
	type[i]='\0';
	while(s[i]==' ' || s[i]=='\t')
	  i++;
	for(j=0 ; i<xs && s[i]!=' ' && s[i] !='\t' && j < 100; i++,j++)
	  value[j]=s[i];
	if(j==100){
	  printf("Error in parameter File - input2 too long!");
	  return NULL;
	}
	value[j]='\0';
	
	if(j!=0){
	  /*printf("set %s to %s, ",type, value);*/
	  /* Check for all possible inputs here */
	  if(strcmp(type,"tabOutFile")==0){
	    free(xcs->tabOutFile);
	    assert(( xcs->tabOutFile = (char *)calloc(strlen(value)+1, sizeof(char)))!=NULL);
	    strcpy(xcs->tabOutFile, value);
	  }else if(strcmp(type,"nrExps")==0){
	    xcs->nrExps = getIntValue(value);
	  }else if(strcmp(type,"initialSeed")==0){
	    xcs->initialSeed  = getIntValue(value);
	  }else if(strcmp(type,"initialFoldSeed")==0){
	    xcs->initialFoldSeed  = getIntValue(value);
	  }else if(strcmp(type,"maxNrSteps")==0){
	    xcs->maxNrSteps = getIntValue(value);
	  }else if(strcmp(type,"do10FoldedCrossValidation")==0){
	    xcs->do10FoldedCrossValidation = getIntValue(value);
	  }else if(strcmp(type,"testFrequency")==0){
	    xcs->testFrequency = getIntValue(value);
	  }else if(strcmp(type,"popOutFrequency")==0){
	    xcs->popOutFrequency = getIntValue(value);
	  }else if(strcmp(type,"doGenerateTree")==0){
	    xcs->doGenerateTree = getIntValue(value);
	  }else if(strcmp(type,"maxPopSize")==0){
	    xcs->maxPopSize = getIntValue(value);
	  }else if(strcmp(type,"alpha")==0){
	    xcs->alpha = getDoubleValue(value);
	  }else if(strcmp(type,"beta")==0){ /* setting all learning rates to beta! */
	    xcs->betaP = getDoubleValue(value);
	    xcs->betaE = getDoubleValue(value);
	    xcs->betaF = getDoubleValue(value);
	    xcs->betaS = getDoubleValue(value);
	  }else if(strcmp(type,"betaP")==0){
	    xcs->betaP = getDoubleValue(value);
	  }else if(strcmp(type,"betaE")==0){
	    xcs->betaE = getDoubleValue(value);
	  }else if(strcmp(type,"betaF")==0){
	    xcs->betaF = getDoubleValue(value);
	  }else if(strcmp(type,"betaS")==0){
	    xcs->betaS = getDoubleValue(value);
	  }else if(strcmp(type,"gamma")==0){
	    xcs->gamma = getDoubleValue(value);
	  }else if(strcmp(type,"epsilon0")==0){
	    xcs->epsilon0 = getDoubleValue(value);
	  }else if(strcmp(type,"nu")==0){
	    xcs->nu = getIntValue(value);
	  }else if(strcmp(type,"fallOffRate")==0){
	    xcs->fallOffRate = getDoubleValue(value);
	  }else if(strcmp(type,"thetaGA")==0){
	    xcs->thetaGA = getIntValue(value);
	  }else if(strcmp(type,"fitnessReduction")==0){
	    xcs->fitnessReduction = getDoubleValue(value);
	  }else if(strcmp(type,"doSetPreErrDirectly")==0){
	    xcs->doSetPreErrDirectly = getIntValue(value);
	  }else if(strcmp(type,"doInitializePE")==0){
	    xcs->doInitializePE = getIntValue(value);
	  }else if(strcmp(type,"doFitnessAdjustment")==0){
	    xcs->doFitnessAdjustment = getIntValue(value);
	  }else if(strcmp(type,"tournamentSize")==0){
	    xcs->tournamentSize = getDoubleValue(value);
	  }else if(strcmp(type,"forceDifferentInTournament")==0){
	    xcs->forceDifferentInTournament = getDoubleValue(value);
	  }else if(strcmp(type,"selectTolerance")==0){
	    xcs->selectTolerance = getDoubleValue(value);
	  }else if(strcmp(type,"crossoverType")==0){
	    xcs->crossoverType = getIntValue(value);
	  }else if(strcmp(type,"chi")==0){
	    xcs->chi = getDoubleValue(value);
	  }else if(strcmp(type,"mu")==0){
	    xcs->mu = getDoubleValue(value);
	  }else if(strcmp(type,"mutateRand")==0){
	    xcs->mutateRand = getDoubleValue(value);
	    free(xcs->mutRandAll);
	    assert((xcs->mutRandAll = (double *)calloc(getConditionLength(), sizeof(double)))!=NULL);
	    for(i=0; i<getConditionLength(); i++) {
	      (xcs->mutRandAll)[i] = xcs->mutateRand*(getUpperBound(i) - getLowerBound(i));
	    }
	  }else if(strcmp(type,"mutRandAll")==0){
	    printf("Direct setting of mut rand all not supported right now !\n");
	  }else if(strcmp(type,"doGeneralizationMutation")==0){
	    xcs->doGeneralizationMutation = getIntValue(value);
	  }else if(strcmp(type,"doNicheMutation")==0){
	    xcs->doNicheMutation = getIntValue(value);
	  }else if(strcmp(type,"doMAM")==0){
	    xcs->doMAM = getIntValue(value);
	  }else if(strcmp(type,"doGradient")==0){
	    xcs->doGradient = getIntValue(value);
	  }else if(strcmp(type,"doRelativeFitness")==0){
	    xcs->doRelativeFitness = getIntValue(value);
	  }else if(strcmp(type,"doGAErrorBasedSelect")==0){
	    xcs->doGAErrorBasedSelect = getIntValue(value);
	  }else if(strcmp(type,"delta")==0){
	    xcs->delta = getDoubleValue(value);
	  }else if(strcmp(type,"thetaDel")==0){
	    xcs->thetaDel = getIntValue(value);
	  }else if(strcmp(type,"deletionType")==0){
	    xcs->deletionType = getIntValue(value);
	  }else if(strcmp(type,"dontCareProb")==0){
	    xcs->dontCareProb = getDoubleValue(value);
	  }else if(strcmp(type,"coverRand")==0){
	    xcs->coverRand = getDoubleValue(value);
	  }else if(strcmp(type,"doGASubsumption")==0){
	    xcs->doGASubsumption = getIntValue(value);
	  }else if(strcmp(type,"doActionSetSubsumption")==0){
	    xcs->doActionSetSubsumption = getIntValue(value);
	  }else if(strcmp(type,"thetaSub")==0){
	    xcs->thetaSub = getIntValue(value);
	  }else if(strcmp(type,"exploreProb")==0){
	    xcs->exploreProb = getDoubleValue(value);
	  }else if(strcmp(type,"teletransportation")==0){
	    xcs->teletransportation = getIntValue(value);
	  }else if(strcmp(type,"initializePopulation")==0){
	    xcs->initializePopulation = getIntValue(value);
	  }else{
	    /* if no string matches try to pass it to the environmental settings */
/*	    if(!setEnvParam(type, getDoubleValue(value))) 
	      printf("Could not resolve line %s\n", s);
	      */
	  }
	}
      }
    }
  }
  return xcs;
}



/* Sets the xcs values to the macro values */
void setMacroValues(struct XCS *xcs)
{
  int i;

  assert(( xcs->tabOutFile = (char *)calloc(strlen(TAB_OUT_FILE)+1, sizeof(char)))!=NULL);
  strcpy(xcs->tabOutFile, TAB_OUT_FILE);
  xcs->nrExps = NR_EXPS;
  xcs->maxNrSteps = MAX_NR_STEPS;
  randomize();
  xcs->initialSeed = getSeed();
  randomize();
  xcs->initialFoldSeed = getSeed();
  xcs->do10FoldedCrossValidation = DO_10FOLDED_CROSS_VALIDATION;
  xcs->testFrequency = TEST_FREQUENCY;
  xcs->popOutFrequency = POP_OUT_FREQUENCY;
  xcs->doGenerateTree = DO_GENERATE_TREE;
  xcs->maxPopSize = MAX_POP_SIZE;
  xcs->alpha = ALPHA;
  xcs->betaP = BETA_P;
  xcs->betaE = BETA_E;
  xcs->betaF = BETA_F;
  xcs->betaS = BETA_S;
  xcs->gamma = GAMMA;
  xcs->epsilon0 = EPSILON_0;
  xcs->nu = (int)NU;
  xcs->fallOffRate = FALL_OFF_RATE;
  xcs->thetaGA = THETA_GA;
  xcs->fitnessReduction = FITNESS_REDUCTION;
  xcs->doSetPreErrDirectly = DO_SET_PRE_ERR_DIRECTLY;
  xcs->doInitializePE = DO_INITIALIZE_P_E;
  xcs->doFitnessAdjustment = DO_FITNESS_ADJUSTMENT;
  xcs->tournamentSize = TOURNAMENT_SIZE;
  xcs->forceDifferentInTournament = FORCE_DIFFERENT_IN_TOURNAMENT;
  xcs->selectTolerance = SELECT_TOLERANCE;
  xcs->crossoverType = CROSSOVER_TYPE;
  xcs->chi = CROSSPX;
  xcs->mu = MUTPMUE;
  xcs->mutateRand = MUTATE_RAND;
  xcs->doGeneralizationMutation = DO_GENERALIZATION_MUTATION;
  xcs->doNicheMutation = DO_NICHE_MUTATION;
  xcs->doMAM = DO_MAM;
  xcs->doGradient = DO_GRADIENT;
  xcs->doRelativeFitness = DO_RELATIVE_FITNESS;
  xcs->doGAErrorBasedSelect = DO_GA_ERROR_BASED_SELECT;
  xcs->delta = DELTA;
  xcs->thetaDel = THETA_DEL;
  xcs->deletionType = DELETION_TYPE;
  xcs->dontCareProb = DONT_CARE_PROB;
  xcs->coverRand = COVER_RAND;
  xcs->doGASubsumption = DO_GA_SUBSUMPTION;
  xcs->doActionSetSubsumption = DO_ACTION_SET_SUBSUMPTION;
  xcs->thetaSub = THETA_SUB;
  xcs->exploreProb = EXPLORE_PROB;
  xcs->teletransportation = TELETRANSPORTATION;
  xcs->initializePopulation = INITIALIZE_POPULATION;

  assert((xcs->mutRandAll = (double *)calloc(getConditionLength(), sizeof(double)))!=NULL);
  for(i=0; i<getConditionLength(); i++) {
    (xcs->mutRandAll)[i] = xcs->mutateRand*(getUpperBound(i) - getLowerBound(i));
  }
}

/* Constructs a new XCS and copies the old parameters */
struct XCS *copyXCS(struct XCS *xcsOld)
{
  struct XCS *xcs;
  int i;

  assert((xcs=( struct XCS *)calloc(1,sizeof(struct XCS)))!=NULL);
  assert(( xcs->tabOutFile = (char *)calloc(strlen(xcsOld->tabOutFile)+1, sizeof(char)))!=NULL);
  strcpy(xcs->tabOutFile, xcsOld->tabOutFile);
  xcs->nrExps = xcsOld->nrExps;
  xcs->maxNrSteps = xcsOld->maxNrSteps;
  xcs->initialSeed = xcsOld->initialSeed;
  xcs->initialFoldSeed = xcsOld->initialFoldSeed;
  xcs->do10FoldedCrossValidation = xcsOld->do10FoldedCrossValidation;
  xcs->testFrequency = xcsOld->testFrequency;
  xcs->popOutFrequency = xcsOld->popOutFrequency;
  xcs->doGenerateTree = xcsOld->doGenerateTree;
  xcs->maxPopSize = xcsOld->maxPopSize;
  xcs->alpha = xcsOld->alpha;
  xcs->betaP = xcsOld->betaP;
  xcs->betaE = xcsOld->betaE;
  xcs->betaF = xcsOld->betaF;
  xcs->betaS = xcsOld->betaS;
  xcs->gamma = xcsOld->gamma;
  xcs->epsilon0 = xcsOld->epsilon0;
  xcs->nu = xcsOld->nu;
  xcs->fallOffRate = xcsOld->fallOffRate;
  xcs->thetaGA = xcsOld->thetaGA;
  xcs->fitnessReduction = xcsOld->fitnessReduction;
  xcs->doSetPreErrDirectly = xcsOld->doSetPreErrDirectly;
  xcs->doInitializePE = xcsOld->doInitializePE;
  xcs->doFitnessAdjustment = xcsOld->doFitnessAdjustment;
  xcs->tournamentSize = xcsOld->tournamentSize;
  xcs->forceDifferentInTournament = xcsOld->forceDifferentInTournament;
  xcs->selectTolerance = xcsOld->selectTolerance;
  xcs->crossoverType = xcsOld->crossoverType;
  xcs->chi = xcsOld->chi;
  xcs->mu = xcsOld->mu;
  xcs->mutateRand = xcsOld->mutateRand;
  assert((xcs->mutRandAll = (double *)calloc(getConditionLength(), sizeof(double)))!=NULL);
  for(i=0; i<getConditionLength(); i++) {
    (xcs->mutRandAll)[i] = xcs->mutateRand*(getUpperBound(i) - getLowerBound(i));
  }
  xcs->doGeneralizationMutation = xcsOld->doGeneralizationMutation;
  xcs->doNicheMutation = xcsOld->doNicheMutation;
  xcs->doMAM = xcsOld->doMAM;
  xcs->doGradient = xcsOld->doGradient;
  xcs->doRelativeFitness = xcsOld->doRelativeFitness;
  xcs->doGAErrorBasedSelect = xcsOld->doGAErrorBasedSelect;
  xcs->delta = xcsOld->delta;
  xcs->thetaDel = xcsOld->thetaDel;
  xcs->deletionType = xcsOld->deletionType;
  xcs->dontCareProb = xcsOld->dontCareProb;
  xcs->coverRand = xcsOld->coverRand;
  xcs->doGASubsumption = xcsOld->doGASubsumption;
  xcs->doActionSetSubsumption = xcsOld->doActionSetSubsumption;
  xcs->thetaSub = xcsOld->thetaSub;
  xcs->exploreProb = xcsOld->exploreProb;
  xcs->teletransportation = xcsOld->teletransportation;
  xcs->initializePopulation = xcsOld->initializePopulation;

  return xcs;
}

void freeXCS(struct XCS *xcs)
{
  free(xcs->tabOutFile);
  free(xcs);
}

/* Prints all XCS parameters */
void fprintXCS(FILE *outfile, struct XCS *xcs)
{
  fprintf(outfile, "# tabOutFile %s nrExps %d maxNrSteps %d initialSeed %d initialFoldSeed %d do10FoldedC.V. %d testFrequency %d popOutFrequency %d doGenerateTree %d maxPopSize %d\n", xcs->tabOutFile, xcs->nrExps, xcs->maxNrSteps, xcs->initialSeed, xcs->initialFoldSeed, xcs->do10FoldedCrossValidation, xcs->testFrequency, xcs->popOutFrequency, xcs->doGenerateTree, xcs->maxPopSize);
  fprintf(outfile, "# alpha %f betaP %f betaE %f betaF %f betaS %f gamma %f epsilon0 %f nu %d fallOffRate %f\n", xcs->alpha, xcs->betaP, xcs->betaE, xcs->betaF, xcs->betaS, xcs->gamma, xcs->epsilon0, xcs->nu, xcs->fallOffRate);
  fprintf(outfile, "# thetaGA %d fitnessReduction %f doSetPreErrDirectly %d doInitializePE %d doFitnessAdjustment %d tournamentSize %4.2f selectTolerance %4.6f forceDifferentInTournament %4.2f crossoverType %d chi %f mu %f mutateRand %f doGeneralizationMutation %d doNicheMutation %d delta %f thetaDel %d deletionType %d \n", xcs->thetaGA, xcs->fitnessReduction, xcs->doSetPreErrDirectly, xcs->doInitializePE, xcs->doFitnessAdjustment, xcs->tournamentSize, xcs->selectTolerance, xcs->forceDifferentInTournament, xcs->crossoverType, xcs->chi, xcs->mu, xcs->mutateRand, xcs->doGeneralizationMutation, xcs->doNicheMutation, xcs->delta, xcs->thetaDel, xcs->deletionType);
  fprintf(outfile,"# doMAM %d doGradient %d doRelativeFitness %d doGAErrorBasedSelect %d\n", xcs->doMAM, xcs->doGradient, xcs->doRelativeFitness, xcs->doGAErrorBasedSelect);
  fprintf(outfile, "# dontCareProb %f coverRand %f doGASubsumption %d doActionSetSubsumption %d thetaSub %d exploreProb %f teletransportation %d initializePopulation %d\n", xcs->dontCareProb, xcs->coverRand, xcs->doGASubsumption, xcs->doActionSetSubsumption, xcs->thetaSub, xcs->exploreProb, xcs->teletransportation, xcs->initializePopulation);
}



