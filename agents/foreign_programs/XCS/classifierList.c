/*
/       (XCS-C 1.1)
/       ------------------------------------
/       Learning Classifier System based on accuracy
/
/     by Martin Butz
/     University of Wuerzburg / University of Illinois at Urbana/Champaign
/     butz@illigal.ge.uiuc.edu
/     Last modified: 09-04-2003
/
/     All actions related to the xClassifier and xClassifierSet.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "xcsMacros.h"
#include "env.h"
#include "xcs.h"
#include "classifierList.h"
#include "actionSelection.h"


/* creates a random classifier set of size maxPopSize*/
struct xClassifierSet * createRandomClassifierSet(int condLength, int maxPopSize, double dontCareProb, int doFitnessAdjustment, int doSetPreErrDirectly)
{
  struct xClassifierSet *head=NULL;
  struct xClassifier *cl;
  int i=0;
  
  for(i=0;i<maxPopSize;i++){
    assert((cl=( struct xClassifier *)calloc(1,sizeof(struct xClassifier)))!=NULL);
    assert((cl->con=(struct xCondition *)calloc(1,sizeof(struct xCondition)))!=NULL);
    
    createRandomCondition(cl->con, condLength, dontCareProb);
    createRandomAction(&(cl->act));
    setInitialVariables(cl , 0, doSetPreErrDirectly);
    addClassifierToSet(cl, &head, doFitnessAdjustment);
  }
  return head;
}

/* Creates a random condition of length condLength choosing DONT_CARE with probability dontCareProb */ 
void createRandomCondition(struct xCondition *con, int condLength, double dontCareProb)
{
  int j;

  if(getGeneralType==0) {
    for(j=0; j<condLength; j++){
      if(urand()>dontCareProb)
        addXCPos(con, j, '0'+(int)(urand()*2.));
    }
  }else{
    printf("Random initialization not supported for other types !!! \n");
    return;
  }
}

/* creates a random action */
void createRandomAction(int *act)
{
  *act = (int)(urand()* getNumberOfActions());
}

/* sets all initial varibles of a classifier list ->used by various functions */
void setInitialVariables(struct xClassifier *cl, int itTime, int doSetPreErrDirectly)
{
  int i;
  double pre=0, mad=0;
  struct condAttribute *conArray;

  cl->pre=getPaymentRange()/2.;
  cl->preer=getPaymentRange()/2.;
  cl->acc=FIT_INI;
  cl->fit=FIT_INI;
  cl->num=1;
  cl->exp=1;
  cl->peerssest=1.;
  cl->gaIterationTime=itTime;

  if(doSetPreErrDirectly) {
    for(i=0; i<2; i++) {
      conArray = getAttributeArray(cl->con);
      pre = getExactEstimate(conArray, cl->act, &mad);
      if(pre>=0) {
        cl->pre = pre;
        cl->preer = mad;
      }
      free(conArray);
    }
  }  
}


/*############################## match set operations #################################*/


struct xClassifierSet *copyClassifierSet(struct xClassifierSet *pop)
{
  struct xClassifierSet *set = NULL;
  for( ; pop!=NULL; pop=pop->next) {
    addNewClassifierToSet(pop->cl, &set);
  }
  return set;
  
}


/**
 * Gets the match-set that matches state from pop.
 * If a classifier was deleted, record its address in killset to be
 * able to update former actionsets.
 * The iteration time 'itTime' is used when creating a new classifier
 * due to covering. Covering occurs when not all possible actions are
 * present in the match set. Thus, it is made sure that all actions
 * are present in the match set.
 */
struct xClassifierSet * getMatchSet(struct XCS *xcs, struct attributes *state, struct xClassifierSet **pop, struct xClassifierSet **killset, int itTime)
{
  struct xClassifierSet *mset=NULL, *poppointer;
  struct xClassifier *killedp, *coverCl;
  int popSize=0, setSize=0, *coveredActions, i, representedActions;
  double spec=0.;

  assert((coveredActions = (int *)calloc(getNumberOfActions(),sizeof(int)))!=0);

  if(getGeneralType() == 0) { /* all boolean entries */
    for(poppointer= *pop; poppointer!=NULL;poppointer=poppointer->next) {
      /* Calculate the mean prediction probability and the population size to detect loops */
      popSize += poppointer->cl->num;
      
      if(matchBoolean(state, poppointer->cl->con)) {
        /* Add matching classifier to the matchset */
        addNewClassifierToSet(poppointer->cl, &mset); 
        setSize+=poppointer->cl->num;
        spec += getSpecificity(poppointer->cl)*poppointer->cl->num;
      }
    }
  }else{
    for(poppointer= *pop; poppointer!=NULL;poppointer=poppointer->next) {
      /* Calculate the mean prediction probability and the population size to detect loops */
      popSize += poppointer->cl->num;
      
      if(match(state, poppointer->cl->con)) {
        /* Add matching classifier to the matchset */
        addNewClassifierToSet(poppointer->cl, &mset); 
        setSize+=poppointer->cl->num;
        spec += getSpecificity(poppointer->cl)*poppointer->cl->num;
      }
    }

  }

  /* Create covering classifiers, if not all actions are covered */
  representedActions = nrActionsInSet(mset, coveredActions);
  
  while( representedActions < getNumberOfActions() ) {
    for(i=0; i<getNumberOfActions(); i++){
      /* make sure that all actions are covered! */
      if(coveredActions[i]==0){
        coverCl=createNewCoverMatch(state, itTime, mset, i, xcs);
        /*printf("Covering... ");
         *printClassifier(coverCl);
         */
        addNewClassifierToSet( coverCl, &mset);
        addNewClassifierToSet( coverCl, pop);
        popSize++;
        setSize++;
        coverCl->peerssest=setSize;
      }
    }
    
    /* Delete classifier if population is too big and record it in killset */
    while( popSize > xcs->maxPopSize ) {
      /* PL */
      killedp = deleteClassifier(pop, xcs);
      if(killedp!=NULL) {
        deleteClassifierPointerFromSet(&mset, killedp);
        addClassifierToPointerSet(killedp, killset);
      }      
      popSize--;
    }
    representedActions = nrActionsInSet(mset, coveredActions);
  }

  free(coveredActions);
  /* return the match set */
  return mset;
}

/**
 * Determines the number of (mikro-)classifiers in set c that are matching the current state.
 */
int getMatchSupport(struct attributes *state, struct xClassifierSet *c)
{
  int ret=0;
  for( ; c!=NULL; c=c->next) {
    /* Calculate the mean prediction probability and the population size to detect loops */
    if(match(state, c->cl->con)) {
      ret += c->cl->num;
    }
  }
  return ret;
}

/**
 * Determines the number of (mikro-)classifiers in set c that are specifying the specified action.
 */
int getActionSupport(int action, struct xClassifierSet *c)
{
  int ret=0;
  for( ; c!=NULL; c=c->next) {
    /* Calculate the mean prediction probability and the population size to detect loops */
    if(c->cl->act == action) {
      ret += c->cl->num;
    }
  }
  return ret;
}

/* Returns the number of actions in the set and stores which actions are covered 
 * in the array coveredActions */
int nrActionsInSet(struct xClassifierSet *set, int *coveredActions)
{
  int nr,i;

  for(i=0; i<getNumberOfActions(); i++) {
    coveredActions[i]=0;
  }
  for(nr=0; nr<getNumberOfActions() && set!=NULL; set=set->next) {
    if(coveredActions[set->cl->act]==0) {
      coveredActions[set->cl->act]=1;
      nr++;
    }
  }
  return nr;
}

/* returns 1 if m matches c */
int matchBoolean(struct attributes *state, struct xCondition *c)
{
  struct xCList *l;
  
  for(l=c->l; l!=NULL; l=l->next) {
    if((l->value).boole != (state[l->pos]).boole && (state[l->pos]).boole !=DONT_CARE)
      return 0;
  }
  return 1;
}

/* returns 1 if m matches c */
int match(struct attributes *state, struct xCondition *c)
{
  struct xCList *l;
  
  for(l=c->l; l!=NULL; l=l->next) {
    if(! doesMatchAttribute(l->value, state[l->pos]))
      return 0;
  }

  return 1;
}

/**
 * returns the relation between the two: 0=equal, 1=more general, 2=more specific, 3=both 
 * Only supports binary problems right now !!!
 */
int compareStateWithClassifier(struct attributes *state, struct xCondition *c)
{
  struct xCList *l;
  int i, moreGeneral=0, moreSpecific=0;

  if(getGeneralType==0) {
    for(i=0, l=c->l; i<getConditionLength(); i++) {
      if((state[i]).boole =='#') {
        while(l!=NULL && l->pos<i)
          l=l->next;
        if(l!=NULL && l->pos==i)
          moreSpecific=1;
      }else{
        while(l!=NULL && l->pos<i)
          l=l->next;
        if(l==NULL || l->pos>i)
          moreGeneral=1;
        else if((state[i]).boole != (l->value).boole )
          return 3;
      }
    }
    if(moreSpecific==0) {
      if(moreGeneral==0)
        return 0;
      else
        return 1;
    }else{
      if(moreGeneral==0)
        return 2;
      else
        return 3;
    }
  }
  return -1;
}

/* creates a matching classifier, deletes a classifier, if necessary,
 * and adds the new classifier to the population; the created
 * classifier matches state and has action "action".
 */
struct xClassifier * createNewCoverMatch(struct attributes *state, int itTime, struct xClassifierSet *set, int action, struct XCS *xcs)
{
  struct xClassifier *cl; 

  /* get memory for the new classifier */
  assert((cl=( struct xClassifier *)calloc(1,sizeof(struct xClassifier)))!=NULL);
  assert((cl->con=(struct xCondition *)calloc(1,sizeof(struct xCondition)))!=NULL);

  /* create condition and action */
  createMatchingCondition(cl->con, state, xcs);
  cl->act = action;

  /* set the parameters to the initial values */
  setInitialVariables(cl, itTime, xcs->doSetPreErrDirectly);

  return cl;
}

/* Creates a condition that matches mstring choosing DONT_CARE with probability dontCareProb */
void createMatchingCondition(struct xCondition *con, struct attributes *state, struct XCS *xcs)
{
  int j;
  int low, up;
  double lowReal, upReal;

 for(j=0; j<getConditionLength(); j++) {
    switch(getAttributeType(j)) {
    case 0:
      if(urand()>xcs->dontCareProb) {
        addXCPos(con, j, state[j].boole);
      }
      break;
    case 1: 
      low = state[j].integer - (int)(urand() * (xcs->coverRand+1.));
      up = state[j].integer + (int)(urand() * (xcs->coverRand+1.));
      if(low < getLowerBound(j))
        low = (int)getLowerBound(j);
      if(up > getUpperBound(j))
        up = (int)getUpperBound(j);
      addXCBounds(con, j, low, up);
      break;
    case 2:
      lowReal = state[j].real - (urand() * xcs->coverRand);
      upReal = state[j].real + (urand() * xcs->coverRand);
      if(lowReal < getLowerBound(j))
        lowReal = getLowerBound(j);
      if(upReal > getUpperBound(j))
        upReal = getUpperBound(j);
      addXCRealBounds(con, j, lowReal, upReal);
      break;
    default:
      printf("Format not supported !!!\n");
      
      break;
    }
  }
}


/*########################### action set operations ####################################*/


/* Get ActionSet out of the set ms (the match set) that only has classifiers that match action,
 * and  increase the experience of those classifiers
 * returns teh created action set */

struct xClassifierSet * getActionSet(int action, struct xClassifierSet *ms)
{
  struct xClassifierSet *aset=NULL;
  
  /* insert all classifiers with action == 'action' to the new xClassifierSet */
  for(; ms!=NULL; ms=ms->next){
    if(action == ms->cl->act){
      addNewClassifierToSet(ms->cl, &aset);
    }
  }

  return aset;
}

/**
 * This is the reinforcement component,
 * If maxP equals 0 then there was no previous action set (single-step
 * env or a new trial) updates: experience (always first!), fitness
 * (second or last), prediction (before or after prediction error),
 * prediction error (before or after prediction), peer set size
 * estimate. 
 */
void adjustActionSet(struct XCS *xcs, struct xClassifierSet **aset, double maxP, double reward, struct xClassifierSet **pop, struct xClassifierSet **killset)
{
  double P, setsize=0, fitSum=0.;
  struct xClassifierSet *setp;  
  
  for(setp=*aset; setp!=NULL; setp=setp->next){
    setp->cl->exp++;
    if(setp->cl->exp>10000)
      setp->cl->exp=10000;
    setsize += setp->cl->num;
    fitSum += setp->cl->fit;
  }
  /* Update the fitness */
  if(DO_FITNESSUPDATE_FIRST)
    updateFitness(*aset, xcs);
  
  /* Update prediction, prediction error and peer set size estimate */
  P=reward+(xcs->gamma)*maxP;
  if(!xcs->doSetPreErrDirectly) {
    for(setp=*aset; setp!=NULL; setp=setp->next) {
      if(UPDATEORDER_PRED_PREDERROR){
        /* update prediction */
        if(xcs->doGradient) {
          setp->cl->pre += (xcs->betaP) * setp->cl->fit * (P - setp->cl->pre) / fitSum;
        }else{
          if(xcs->doMAM>0 && (double)setp->cl->exp < 1./xcs->betaP) {
            setp->cl->pre= (setp->cl->pre* ((double)setp->cl->exp - 1.) + P) / (double)setp->cl->exp;
          }else{
            setp->cl->pre += (xcs->betaP) * (P - setp->cl->pre);        
          }
        }
        /* update prediction error */
        if(xcs->doMAM==1 && (double)setp->cl->exp < 1./xcs->betaE) {
          setp->cl->preer= (setp->cl->preer* ((double)setp->cl->exp - 1.) + absDouble(P - setp->cl->pre)) / (double)setp->cl->exp;
        }else{
          setp->cl->preer += (xcs->betaE) * (absDouble(P - setp->cl->pre) - setp->cl->preer);
        }
      }else{
        /* update prediction error */
        if(xcs->doMAM==1 && (double)setp->cl->exp < 1./xcs->betaE) {
          setp->cl->preer= (setp->cl->preer* ((double)setp->cl->exp - 1.) + absDouble(P - setp->cl->pre)) / (double)setp->cl->exp;
        }else{
          setp->cl->preer += (xcs->betaE) * (absDouble(P - setp->cl->pre) - setp->cl->preer);
        }
        /* update prediction */
        if(xcs->doGradient) {
          setp->cl->pre += (xcs->betaP) * setp->cl->fit * (P - setp->cl->pre) / fitSum;
        }else{
          if(xcs->doMAM>0 && (double)setp->cl->exp < 1./xcs->betaP) {
            setp->cl->pre= (setp->cl->pre* ((double)setp->cl->exp - 1.) + P) / (double)setp->cl->exp;
          }else{
            setp->cl->pre += (xcs->betaP) * (P - setp->cl->pre);        
          }      
        }
      }
      if(xcs->doMAM>0 && (double)setp->cl->exp < 1./xcs->betaS) {
        setp->cl->peerssest=(setp->cl->peerssest*((double)(setp->cl->exp-1))+setsize)/(double)setp->cl->exp; 
      }else{
        setp->cl->peerssest+= (xcs->betaS) * (setsize-setp->cl->peerssest);
      }
    }
  }
  /* Update the fitness */
  if(!DO_FITNESSUPDATE_FIRST)
    updateFitness(*aset, xcs);

  if( xcs->doActionSetSubsumption )
    doActionSetSubsumption(aset, pop, killset, xcs->thetaSub, xcs->doFitnessAdjustment);
}


/* update the fitnesses of an action set 
 * (the previous [A] in multi-step envs or the current [A] in single-step envs.) */
void updateFitness(struct xClassifierSet *aset, struct XCS *xcs)
{
  struct xClassifierSet *setp;
  double ksum=0, powerSub;
  int i;

  /* If the action set got NULL (due to deletion) return */
  if(aset==NULL)
    return;
  
  /* Calculate the accuracies of all classifiers and get the sum of all accuracies */
  for(setp=aset;setp!=NULL;setp=setp->next) {
    if(setp->cl->preer <= xcs->epsilon0*(double)getPaymentRange()) {
      setp->cl->acc=1.0;
    }else{
      powerSub = setp->cl->preer/(xcs->epsilon0*(double)getPaymentRange());
      setp->cl->acc = powerSub;
      for(i=2; i <= xcs->nu; i++)
        setp->cl->acc *= powerSub;
      setp->cl->acc = xcs->alpha * (1/(setp->cl->acc));
    }
    ksum += setp->cl->acc*(double)setp->cl->num;
  }
  
  if(xcs->doRelativeFitness) {
    /* Update the fitnesses of the classifiers in aset using the relative accuracy values */
    for(setp=aset; setp!=NULL; setp=setp->next) {
      setp->cl->fit += xcs->betaF * ((setp->cl->acc * setp->cl->num) / ksum - setp->cl->fit);
    }
  }else{
    /* Update the fitnesses of the classifiers in aset the actual accuracy values */
    for(setp=aset; setp!=NULL; setp=setp->next) {
      setp->cl->fit += xcs->betaF * ( (setp->cl->acc * setp->cl->num) - setp->cl->fit);
    }
  }
}



/*############################### discovery mechanism ##################################*/


/* the discovery conmponent with the genetic algorithm 
 * note: some classifiers in set could be deleted ! */
void discoveryComponent(struct xClassifierSet **set, struct xClassifierSet **pop, struct xClassifierSet **killset, int itTime, struct attributes *state, struct XCS *xcs, double currentReward)
{
  struct xClassifierSet *setp;
  struct xClassifier *cl[2], *parents[2];
  double fitsum=0., errsum=0., pre, mad;
  int i, len, gaitsum=0, setsum=0, expsum=0;
  struct condAttribute *conArray;

  /* if the classifier set is empty, return (due to deletion) */
  if(*set==NULL)
    return;
  
  /* get all sums that are needed to do the discovery */
  getDiscoversSums(*set,&fitsum,&gaitsum,&setsum,&expsum,&errsum);
  
  /* do not do a GA if the average number of time-steps in the set since the last GA
   * is less or equal than thetaGA */
  if( itTime - (double)gaitsum / (double)setsum < xcs->thetaGA)
    return;
  
  setTimeStamps(*set, itTime);
  
  /* select two classifiers (roulette wheel selection) and copy them */
  selectTwoClassifiers(cl, parents, *set, fitsum, setsum, xcs);
     
  /*cl[0]->preer += 10.;
   *cl[1]->preer += 10.;
   */

  /*errorIncrease = xcs->epsilon0 * (double)getPaymentRange() * (pow(0.1,(1./xcs->nu))-1) / pow((0.1*cl[0]->fit),(1/xcs->nu));
  *cl[0]->preer += errorIncrease;
  *errorIncrease = xcs->epsilon0 * (double)getPaymentRange() * (pow(0.1,(1./xcs->nu))-1) / pow((0.1*cl[1]->fit),(1/xcs->nu));
  *cl[1]->preer += errorIncrease;
  */
  /* Reducing fitness of new classifiers by a given factor */
  cl[0]->fit *= xcs->fitnessReduction; /* reduce fitness of new classifiers */
  cl[1]->fit *= xcs->fitnessReduction; /* reduce fitness of new classifiers */

  /* do crossover on the two selected classifiers */
  if(crossover(cl, xcs->crossoverType, xcs->chi)) {
    /* if crossover got applied and changed classifiers, average their values */
    cl[0]->fit   = (cl[0]->fit + cl[1]->fit) / 2.; 
    cl[1]->fit   = cl[0]->fit;

    cl[0]->pre   = (cl[0]->pre + cl[1]->pre) / 2.; 
    cl[0]->preer = (cl[0]->preer + cl[1]->preer) / 2.;
    cl[1]->pre   = cl[0]->pre;
    cl[1]->preer = cl[0]->preer;
  }

  /* do mutation */
  for(i=0; i<2; i++) {
    mutation(cl[i], state, xcs);
  }
  
  /* set prediction and error to current prediction and average error in action set dependent on setting */
  if(xcs->doInitializePE) {
    if(xcs->doInitializePE%2==1) {
      cl[0]->pre = currentReward;
      cl[1]->pre = currentReward;
    }
    if(xcs->doInitializePE>1) {
      cl[0]->preer = errsum/(double)setsum;
      cl[1]->preer = errsum/(double)setsum;

    }
  }

  /* set prediction and error to their actual value if seleted and available */
  if(xcs->doSetPreErrDirectly) {
    for(i=0; i<2; i++) {
      conArray = getAttributeArray(cl[i]->con);
      pre = getExactEstimate(conArray, cl[i]->act, &mad);
      if(pre>=0) {
        cl[i]->pre = pre;
        cl[i]->preer = mad;
      }
      free(conArray);
    }
  }

  /* get the length of the population to check if clasifiers have to be deleted */
  for(len=0, setp=*pop; setp!=NULL; setp=setp->next)
    len += setp->cl->num;
  
  /* insert the new two classifiers and delete two if necessary */
  insertDiscoveredClassifier(cl, parents, set, pop, killset, len, xcs);
}

/**
 * Returns the number of micro classifiers in the population.
 */
int getNumerositySum(struct xClassifierSet *set) 
{
  int ret=0;
  for( ; set!=NULL; set=set->next) {
    ret += set->cl->num;
  }
  return ret;

}

/* Calculate all necessary sums in the set for the discovery component */
void getDiscoversSums(struct xClassifierSet *set, double *fitsum, int *gaitsum, int *setsum, int *expSum, double *errSum)
{
  struct xClassifierSet *setp;
  
  *fitsum=0.;
  *gaitsum=0;
  *setsum=0;  
  *expSum=0;
  *errSum=0;
  for(setp=set;setp!=NULL;setp=setp->next){
    (*fitsum)+=setp->cl->fit;
    (*gaitsum)+=(setp->cl->gaIterationTime)*setp->cl->num;
    (*setsum)+=setp->cl->num;
    (*expSum)+=setp->cl->exp*setp->cl->num;
    (*errSum) += setp->cl->preer*setp->cl->num;
  }
}

/* sets the time steps of all classifiers in the set to itTime (because a GA application is occurring in this set!) */
void setTimeStamps(struct xClassifierSet *set, int itTime)
{
  for( ; set!=NULL; set=set->next){
    set->cl->gaIterationTime=itTime;
  }
}


/*############################ selection mechanisms ####################################*/


/* select two classifiers and copy them as offspring */
void selectTwoClassifiers(struct xClassifier **cl, struct xClassifier **parents, struct xClassifierSet *set, double fitsum, int setsum, struct XCS *xcs)
{
  int i; /*,j, length;*/
  struct xClassifier *clp;

  assert(set!=NULL);

  for(i=0;i<2;i++) {
    if(xcs->tournamentSize<=0)
      clp = selectClassifierUsingRWS(set,fitsum);
    else
      if(i==0)
        clp = selectClassifierUsingTournamentSelection(set, setsum, xcs, 0);
      else
        clp = selectClassifierUsingTournamentSelection(set, setsum, xcs, parents[0]);
    parents[i]=clp;
      
    assert((cl[i]=(struct xClassifier *)calloc(1,sizeof(struct xClassifier)))!=NULL);
    cl[i]->con = copyXC(clp->con);
    cl[i]->act = clp->act;
      
    cl[i]->pre = clp->pre;
    cl[i]->preer = clp->preer;
    cl[i]->acc = clp->acc;
    cl[i]->fit = clp->fit / (double)clp->num;
    cl[i]->num = 1;
    cl[i]->exp = 1;
    cl[i]->peerssest = clp->peerssest;
    cl[i]->gaIterationTime = clp->gaIterationTime;
  }
}

struct xClassifier * selectClassifierUsingTournamentSelection(struct xClassifierSet *set, int setsum, struct XCS *xcs, struct xClassifier *notMe)
{
  struct xClassifierSet *setp, *winnerSet=NULL;
  struct xClassifier *winner=NULL;
  double fitness=-1, value=0;
  int i, j, *sel, size=0;

  assert(set!=NULL);/* there must be at least one classifier in the set */
  if(notMe!=0) {
    if(urand() < xcs->forceDifferentInTournament)
      setsum -= notMe->num; 
    else
      notMe=0; /* picking the same guy is allowed */
  }
  if(setsum==0) /* only one classifier in set */
    return set->cl;

  if(xcs->tournamentSize>1) {/* tournament with fixed size */
    assert((sel = (int *)calloc(setsum, sizeof(int)))!=NULL);
    for(i=0; i<xcs->tournamentSize; i++) { /* (with replacement) */
      sel[(int)(urand()*setsum)]=1;
    }
    if(i-xcs->tournamentSize != 0 && urand() > i-xcs->tournamentSize) 
      /* possible probabilistic selection of the last guy */
      sel[(int)(urand()*setsum)]=1;

    for(setp=set, i=0; setp!=NULL; setp=setp->next) {
      if(setp->cl != notMe) {
        if(fitness < setp->cl->fit/setp->cl->num) {
          for(j=0; j<setp->cl->num; j++) {
            if(sel[i+j]) {
              freeSet(&winnerSet);
              addClassifierToPointerSet(setp->cl, &winnerSet);
              fitness = setp->cl->fit/setp->cl->num;
              break; /* go to next classifier since this one is already a winner*/
            }
          }
        }
        i += setp->cl->num;
      }
    }
    free(sel);
    assert(winnerSet!=NULL);
    size=1;
  }else{
    /* tournament selection with the tournament size approx. equal to tournamentSize*setsum */
    winnerSet=NULL;
    while(winnerSet==NULL) {
      size=0;
      for(setp=set; setp!=NULL; setp=setp->next) {
        if(setp->cl != notMe) { /* do not reselect the same classifier -> this only applies if forcedDifferentInTournament is set!*/

          value = setp->cl->fit/setp->cl->num;
          if(xcs->doGAErrorBasedSelect)
            value = setp->cl->preer;

          if(winnerSet==NULL 
             || (!xcs->doGAErrorBasedSelect && fitness - xcs->selectTolerance <= value)
             || (xcs->doGAErrorBasedSelect && fitness + xcs->selectTolerance * getPaymentRange() >= value)) {
            /* if his fitness is worse then do not bother */
            for(i=0; i<setp->cl->num; i++) {
              if(urand() < xcs->tournamentSize) {
                /* this classifier is a selection candidate and 
                 * his fitness/error is higher/lower or similar to the other classifier */
                if(winnerSet==NULL) {
                  /* the first guy in the tournament */
                  addClassifierToPointerSet(setp->cl, &winnerSet);
                  fitness = value;
                  size=1;
                }else{
                  /* another guy in the tournament */
                  if( (!xcs->doGAErrorBasedSelect && fitness + xcs->selectTolerance >= value)
                      || 
                      (xcs->doGAErrorBasedSelect && fitness - xcs->selectTolerance * getPaymentRange() <= value)) {
                    /* both classifiers in tournament have a similar fitness/error */
                    size += addClassifierToPointerSet(setp->cl, &winnerSet);
                  }else{
                    /* new classifier in tournament is clearly better */
                    freeSet(&winnerSet);
                    winnerSet=NULL;
                    addClassifierToPointerSet(setp->cl, &winnerSet);
                    fitness = value;
                    size=1;
                  }
                }
                break; /* go to next classifier since this one is already a winner*/
              }
            }
          }
        }
      }
    }    
  }

  size = (int)(urand()*size);
  for(setp=winnerSet; setp!=NULL; setp=setp->next) {
    if(size==0)
      break;
    size--;
  }
  winner = setp->cl;
  freeSet(&winnerSet);
  return winner;
}


/* select classifier for the discovery mechanism using roulette wheel selection */
struct xClassifier * selectClassifierUsingRWS(struct xClassifierSet *set,double fitsum)
{
  struct xClassifierSet *setp;
  double choicep;  

  choicep=urand()*fitsum;
  setp=set;
  fitsum=setp->cl->fit;
  while(choicep>fitsum){
    setp=setp->next;
    fitsum+=setp->cl->fit;
  }

  return setp->cl;
}


/*############################# crossover & mutation ###################################*/


/* determines if crossover is applied and calls then the selected crossover type
 * returns if the conditions changed */
int crossover(struct xClassifier **cl, int crossoverType, double chi)
{
  int changed=0;

  if(urand()<chi){
    if(crossoverType == 0)
      changed = uniformCrossover(cl);
    else if(crossoverType == 1)
      changed = onePointCrossover(cl);
    else if(crossoverType ==2)
      changed = twoPointCrossover(cl);
    else
      changed = informedCrossover(cl);
  }
  return changed;
}

/* crosses the two received classifiers using uniform crossover
 * returns if the conditions were changed */
int uniformCrossover(struct xClassifier **cl)
{
  int len, changed =0;
  struct condAttribute help;
  struct xCList *l1=NULL, *l2=NULL, *ll1=NULL, *ll2=NULL;
  double swap;

  len=getConditionLength();
  
  l1=cl[0]->con->l;
  l2=cl[1]->con->l; 
  while(l1!=NULL) {
    while(l2!=NULL && l2->pos < l1->pos) {
      if(urand()<0.5) { /* add l2 to cl[0] and remove it from cl[1] */
        changed = 1;
        /*...removing*/
        if(ll2==NULL) { /* first attribute*/
          cl[1]->con->l = l2->next;
        }else{
          ll2->next = l2->next;
        }
        /*...adding*/
        if(ll1==NULL) { /* first attribute */
          cl[0]->con->l = l2;
        }else{
          ll1->next = l2;
        }
        l2->next = l1;
        ll1=l2;
        /* set l2 */
        if(ll2==NULL) { /* removed first attribute */
          l2=cl[1]->con->l;
        }else{
          l2=ll2->next;
        }
      }else{ /* move l2 to the next position without crossing*/
        ll2=l2;
        l2=l2->next;
      }
    }
    if(l2!=NULL && l2->pos == l1->pos) {
      switch( (l1->value).type) {
      case 0:
	if(urand()<0.5) { /* swap the characters of the two */
	  if( (l1->value).type == 0 && (l1->value).boole == (l2->value).boole ) {
	    ;
	  }else{
	    /*printf("Attributes are not equal during crossover!?\n");*/
	    changed=1;
	    help = l1->value;
	    l1->value=l2->value;
	    l2->value=help;
	  }
	}
	break;
      case 1:
	if(urand()<0.5) { /* swap lower bound value */
	  swap = (l1->value).lowerBound;
	  (l1->value).lowerBound = (l2->value).lowerBound; 
	  (l2->value).lowerBound = swap;
	}
	if(urand()<0.5) {/* swap upper bound value */
	  swap = (l1->value).upperBound;
	  (l1->value).upperBound = (l2->value).upperBound; 
	  (l2->value).upperBound = swap;	  
	}
	if((l1->value).lowerBound > (l1->value).upperBound) {
	  swap = (l1->value).lowerBound;
	  (l1->value).lowerBound = (l1->value).upperBound;
	  (l1->value).upperBound = swap;
	}
	if((l2->value).lowerBound > (l2->value).upperBound) {
	  swap = (l2->value).lowerBound;
	  (l2->value).lowerBound = (l2->value).upperBound;
	  (l2->value).upperBound = swap;
	}
	break;
      case 2:
	if(urand()<0.5) { /* swap lower bound value */
	  swap = (l1->value).lowerRealBound;
	  (l1->value).lowerRealBound = (l2->value).lowerRealBound; 
	  (l2->value).lowerRealBound = swap;
	}
	if(urand()<0.5) {/* swap upper bound value */
	  swap = (l1->value).upperRealBound;
	  (l1->value).upperRealBound = (l2->value).upperRealBound; 
	  (l2->value).upperRealBound = swap;	  
	}
	if((l1->value).lowerRealBound > (l1->value).upperRealBound) {
	  swap = (l1->value).lowerRealBound;
	  (l1->value).lowerRealBound = (l1->value).upperRealBound;
	  (l1->value).upperRealBound = swap;
	}
	if((l2->value).lowerRealBound > (l2->value).upperRealBound) {
	  swap = (l2->value).lowerRealBound;
	  (l2->value).lowerRealBound = (l2->value).upperRealBound;
	  (l2->value).upperRealBound = swap;
	}
	break;
      default:
	printf("Uniform crossover not supported for this format!\n");
	break;
      }
      ll1=l1;
      ll2=l2;
      l1=l1->next;
      l2=l2->next;
    }else if(l2==NULL || l2->pos > l1->pos) {
      if(urand()<0.5) {/* add l1 to cl[1] and remove it from cl[0] */
        changed=1;
        /*...removing */
        if(ll1==NULL) {/* first attribute */
          cl[0]->con->l = l1->next;
        }else{
          ll1->next = l1->next;
        }
        /*...adding */
        if(ll2==NULL) { /* first attribute */
          cl[1]->con->l = l1;
        }else{
          ll2->next = l1;
        }
        l1->next = l2;
        ll2=l1;
        /* set l1 */
        if(ll1==NULL) { /* removed first attribute */
          l1=cl[0]->con->l;
        }else{
          l1=ll1->next;
        }
      }else{ /* move l1 to the next position without crossing*/
        ll1=l1;
        l1=l1->next;
      }
    }
  }
  /* finally, possible last elements in l2 might be crossed */
  while(l2!=NULL) { /* l1=NULL now!!! */
    if(urand()<0.5) {/*add l2 to cl[0] and remove it from cl[1] */
      changed=1;
      /*...removing*/
      if(ll2==NULL) {/* first attribute*/
        cl[1]->con->l = l2->next;
      }else{
        ll2->next = l2->next;
      }
      /*...adding*/
      if(ll1==NULL) { /* first attribute */
        cl[0]->con->l = l2;
      }else{
        ll1->next = l2;
      }
      l2->next = l1;
      ll1=l2;
      /* set l2 */
      if(ll2==NULL) { /* removed first attribute */
        l2=cl[1]->con->l;
      }else{
        l2=ll2->next;
      }
    }else{ /* move l2 to the next position without crossing*/
      ll2=l2;
      l2=l2->next;
    }
  }
  resetXCSize(cl[0]->con);
  resetXCSize(cl[1]->con);
  return changed;
}


/* crosses the two received classifiers using one-point crossover
 * returns if the conditions were changed */
int onePointCrossover(struct xClassifier **cl)
{
  int sep,len;
  struct xCList *l1=NULL, *l2=NULL, *ll1=NULL, *ll2=NULL;
  
  /* get crossing site */
  len=getConditionLength();
  sep=urand()*(len-1);
  /* find cross start */
  for(l1=cl[0]->con->l; l1!=NULL; l1=l1->next) {
    if(l1->pos > sep)
      break;
    ll1=l1;
  }
  for(l2=cl[1]->con->l; l2!=NULL; l2=l2->next) {
    if(l2->pos > sep)
      break;
    ll2=l2;
  }
  if( (ll1==NULL && ll2==NULL) || (l1==NULL && l2==NULL))
    return 0;
  /* cross those... */
  if(ll1==NULL) {
    cl[0]->con->l=l2;
  }else{
    ll1->next=l2;
  }
  if(ll2==NULL) {
    cl[1]->con->l=l1;
  }else{
    ll2->next = l1;
  }
  resetXCSize(cl[0]->con);
  resetXCSize(cl[1]->con);
  return 1;
}


/* crosses the two received classifiers using two-point crossover
 * returns if the conditions were changed */
int twoPointCrossover(struct xClassifier **cl)
{
  int sep[2],len, h, i;
  struct xCList *l1=NULL, *l2=NULL, *ll1=NULL, *ll2=NULL;

  /* get two crossing sites with sep1<sep2 */
  len=getConditionLength();
  sep[0]=urand()*len;
  sep[1]=urand()*len +1;
  if(sep[0]>sep[1]){
    h=sep[0];
    sep[0]=sep[1];
    sep[1]=h;
  }else if(sep[0]==sep[1]){
    sep[1]++;
  }

  for(i=0; i<2; i++) {  /* I do two crosses starting both times from the beginning... */
    /* find cross start */
    ll1=NULL;
    for(l1=cl[0]->con->l; l1!=NULL; l1=l1->next) {
      if(l1->pos >= sep[i])
        break;
      ll1=l1;
    }
    ll2=NULL;
    for(l2=cl[1]->con->l; l2!=NULL; l2=l2->next) {
      if(l2->pos >= sep[i])
        break;
      ll2=l2;
    }
    /* cross those... */
    if(ll1==NULL) {
      cl[0]->con->l=l2;
    }else{
      ll1->next = l2;
    }
    if(ll2==NULL) {
      cl[1]->con->l=l1;
    }else{
      ll2->next = l1;
    }
  }
  resetXCSize(cl[0]->con);
  resetXCSize(cl[1]->con);
  
  return 1;
}

/**
 * Crosses only on certain positions specified in the getCrossPoints function.
 * Right now the specified intervals count.
 */
int informedCrossover(struct xClassifier **cl)
{
  int i, *pos, sep, crosses, crossPos;
  struct xCList *l1=NULL, *l2=NULL, *ll1=NULL, *ll2=NULL;

  assert((pos = (int *)calloc(getConditionLength()+1, sizeof(int)))!=NULL);
  if(! (sep=getCrossPoints(pos)) ) {
    free(pos);
    return uniformCrossover(cl);
  }
  
  crosses = getConditionLength()/sep;
  for(i=1; i<crosses; i++) {  /* I do crosses starting each time from the beginning... */
    if(urand() <0.5) {
      crossPos = i*sep;
      /* find cross start */
      ll1=NULL;
      for(l1=cl[0]->con->l; l1!=NULL; l1=l1->next) {
	if(l1->pos >= crossPos)
	  break;
	ll1=l1;
      }
      ll2=NULL;
      for(l2=cl[1]->con->l; l2!=NULL; l2=l2->next) {
	if(l2->pos >= crossPos)
	  break;
	ll2=l2;
      }
      /* cross those... */
      if(ll1==NULL) {
	cl[0]->con->l=l2;
      }else{
	ll1->next = l2;
      }
      if(ll2==NULL) {
	cl[1]->con->l=l1;
      }else{
	ll2->next = l1;
      }
    }
  }
  resetXCSize(cl[0]->con);
  resetXCSize(cl[1]->con);
  
  free(pos);
  return 1; 
}


/**
 * Calculates the specificity of the classifier
 */
double getSpecificity(struct xClassifier *cl)
{
  double spec;
  struct xCList *l;

  if(getGeneralType==0)
    return cl->con->size;
  
  spec=0;
  for(l=cl->con->l; l!=NULL; l=l->next) 
  {
    switch((l->value).type) {
    case 0:
      spec +=1;
      break;
    case 1:
      spec += (1. - ((l->value).upperBound - (l->value).lowerBound)/(getUpperBound(l->pos) - getLowerBound(l->pos)));
      break;
    case 2:
      spec += (1. - ((l->value).upperRealBound - (l->value).lowerRealBound)/(getUpperBound(l->pos) - getLowerBound(l->pos)));
      break;
    default:
      printf("specificity of this format is not determinable\n");
      break;
    }
  }
  return spec;
}


/* mutate one classifier */
int mutation(struct xClassifier *cl, struct attributes *state, struct XCS *xcs)
{
  int mp, len, mttr=0, lowBound, upBound;
  struct xCondition *con;
  struct xCList *l, *ll=NULL, *newl;
  double help;

  len = getConditionLength();
  con = cl->con;
  l = con->l;
  for(mp=0; mp<len; mp++) {
    if(urand() < xcs->mu) {
      for(; l!=NULL && l->pos < mp; l=l->next) /* move to position */
        ll=l;
      switch( getAttributeType(mp)) {
      case 0: 
        if(l!=NULL && l->pos==mp) { /* mutate specified position */
          if(mutate(&((l->value).boole), (state[l->pos]).boole, xcs->doGeneralizationMutation, xcs->doNicheMutation)) {
            mttr = 1;
            if((l->value).boole == DONT_CARE) {/* remove don't care entry!!! */
              con->size --; /* generalizing */
              if(ll==NULL) {/* remove first */
                con->l = con->l->next;
              }else{
                ll->next=ll->next->next;
              }
              l->next=NULL;
              free(l);
              if(ll==NULL) {
                l = con->l;
              }else{
                l = ll->next;
              }
            }
          }
        }else{ /* add position */
          assert((newl=(struct xCList *)calloc(1,sizeof(struct xCList)))!=0);
          (newl->value).type = 0;
          (newl->value).boole = DONT_CARE;
          newl->pos = mp;
          newl->next = l;
          if( ! mutate(&((newl->value).boole), (state[mp]).boole, xcs->doGeneralizationMutation, xcs->doNicheMutation) ) {
            /* still a don't care symbol -> delete it! */
            free(newl);
          }else{
            con->size++;
            mttr=1;
            if(ll==NULL) { /* add before first one */
              con->l=newl;
            }else{
              ll->next=newl;
            }
            ll = newl;
          }
        }
        break;
        
      case 1: /* integer mutation */
	lowBound = getLowerBound(l->pos);
	upBound = getUpperBound(l->pos);
	if(upBound - lowBound == 1) {
	  /* simulated binary mutation */
	  if( (l->value).lowerBound == upBound) {
	    /* one only */
	    if(urand() < 0.5) {
	      (l->value).lowerBound = (l->value).upperBound = lowBound;
	    }else{
	      (l->value).lowerBound = lowBound;
	      (l->value).upperBound = upBound;
	    }
	  }else if( (l->value).upperBound == lowBound) {
	    /* zero only */
	    if(urand() < 0.5) {
	      (l->value).lowerBound = (l->value).upperBound = upBound;
	    }else{
	      (l->value).lowerBound = lowBound;
	      (l->value).upperBound = upBound;
	    }
	  }else{
	    if(urand() < 0.5) {
	      (l->value).lowerBound = (l->value).upperBound = lowBound;	      
	    }else{	      
	      (l->value).lowerBound = (l->value).upperBound = upBound;
	    }
	  }
	}else if(l->pos==mp) {
          help=1;
          if(urand() <0.5) 
            help = -1.;
          (l->value).lowerBound += 1 + help * (urand() * (xcs->mutRandAll)[mp]);
          help=1;
          if(urand() <0.5) 
            help = -1.;
          (l->value).upperBound += 1 + help * (urand() * (xcs->mutRandAll)[mp]);
          if((l->value).lowerBound > (l->value).upperBound) { /* make sure lower is smaller than upper entry */
            help = (l->value).lowerBound;
            (l->value).lowerBound = (l->value).upperBound;
            (l->value).upperBound = help;
          }
          if((l->value).lowerBound < lowBound)
            (l->value).lowerBound =  lowBound;
          if((l->value).upperBound > upBound)
            (l->value).upperBound = upBound;
        }else{
          printf("Do not support empty entries in this attribute!!!\n");
        }
        break;

      case 2:
        if(l!=NULL && l->pos==mp) {
          help=1;
	  if(urand() <0.5) 
	    help = -1.;
	  (l->value).lowerRealBound += help * (urand() * (xcs->mutRandAll)[mp]);
	  help=1;
	  if(urand() <0.5) 
	    help = -1.;
	  (l->value).upperRealBound += help * (urand() * (xcs->mutRandAll)[mp]);
	  /*(l->value).lowerRealBound += nrand() * cl->preer;
	   *(l->value).upperRealBound += nrand() * cl->preer;
	   */
          if((l->value).lowerRealBound > (l->value).upperRealBound) { /* make sure lower is smaller than upper entry */
            help = (l->value).lowerRealBound;
            (l->value).lowerRealBound = (l->value).upperRealBound;
            (l->value).upperRealBound = help;
          }
          if((l->value).lowerRealBound < getLowerBound(l->pos))
            (l->value).lowerRealBound =  getLowerBound(l->pos);
          if((l->value).upperRealBound > getUpperBound(l->pos))
            (l->value).upperRealBound = getUpperBound(l->pos);
        }else{
          printf("Do not support empty entries in this attribute!!!\n");
        }
        break;
        
      default:
        printf("Format not yet supported !!!\n");
        break;
      }
    }
  }
  len = con->size;
  /*
   * resetXCSize(con);
   * if(len != con->size)
   * printf("Error in Specificity!!! %d != %d", len, con->size);
   */
  if( urand() < xcs->mu ) {
    mttr=1;
    mutateAction(&(cl->act));
  }
  
  /* return if the condition or action of the classifier changed */
  return mttr;
}

/* mutates one position in the condition of a classifier
 * returns if the position really changed (for the GERERALIZATION_MUTATION) */
int mutate(char *c, char s, int doGeneralizationMutation, int doNicheMutation)
{
  int changed=1;
  /* mutate condition -> 0, 1, or DONT_CARE */
  switch(*c) {
  case '0':
    if(doGeneralizationMutation || doNicheMutation ) {
      /* generalize */
      (*c)=DONT_CARE;
    }else{
      if(urand()<0.5)
        (*c)='1';
      else
        (*c)=DONT_CARE;
    }
    break;
  case '1':
    if(doGeneralizationMutation || doNicheMutation ) {
      (*c)=DONT_CARE;
      /* generalize */
    }else{
      if(urand()<0.5)
        (*c)='0';
      else
        (*c)=DONT_CARE;
    }
    break;
  case DONT_CARE:
    if(!doGeneralizationMutation) {
      /* specialize */
      if(doNicheMutation){
        (*c)=s;
      }else{
        if(urand()<0.5)
          (*c)='0';
        else
          (*c)='1';
      }
    }else{
      changed=0;
    }
    break;
  default:
    printf("Mistake during mutation! condition:%c char:%c\n",*c ,s);
    break;
  }
  return changed;
}

void mutateAction(int *action)
{
  int i;

  do{
    i = (int)(urand() * getNumberOfActions());
  }while(*action == i);
  *action = i;
}


/*############################ offspring insertion #####################################*/


/* insert a discovered classifier into the population and respects the population size */
void insertDiscoveredClassifier(struct xClassifier **cl, struct xClassifier **parents, 
                                struct xClassifierSet **set, struct xClassifierSet **pop, 
                                struct xClassifierSet **killset, int len, struct XCS *xcs)
{
  struct xClassifier *killedp;

  len+=2;
  if(xcs->doGASubsumption){
    subsumeClassifier(cl[0], parents, *set, pop, xcs->maxPopSize, xcs->thetaSub, xcs->doFitnessAdjustment);
    subsumeClassifier(cl[1], parents, *set, pop, xcs->maxPopSize, xcs->thetaSub, xcs->doFitnessAdjustment);
  }else{
    addClassifierToSet(cl[0], pop, xcs->doFitnessAdjustment);
    addClassifierToSet(cl[1], pop, xcs->doFitnessAdjustment);
  }
  
  while(len > xcs->maxPopSize) {
    len--;
    killedp=deleteClassifier(pop, xcs);
    
    /* record the deleted classifier to update other sets */
    if(killedp!=NULL) {
      addClassifierToPointerSet(killedp,killset);
      /* update the set */
      updateSet(set, *killset);
    }
  }
}


/*######################## subsumption deletion ########################################*/


/* Action set subsumption as described in the algorithmic describtion of XCS */
void doActionSetSubsumption(struct xClassifierSet **aset, struct xClassifierSet **pop, struct xClassifierSet **killset, int thetaSub, int doFitnessAdjustment)
{
  struct xClassifier *subsumer=NULL;
  struct xClassifierSet *setp, *setpl;
  
  /* Find the most general subsumer */
  for(setp=*aset; setp!=NULL; setp=setp->next) {
    if(isSubsumer(setp->cl, thetaSub)) {
      if(subsumer==NULL || isMoreGeneral(setp->cl->con, subsumer->con)) {
        subsumer = setp->cl;
      }
    }
  }

  /* If a subsumer was found, subsume all classifiers that are more specific. */
  if(subsumer!=NULL) {
    for(setp=*aset, setpl=*aset; setp!=NULL; setp=setp->next) {
      while(isMoreGeneral(subsumer->con, setp->cl->con)) {
        if(doFitnessAdjustment) {
          subsumer->fit += (double)setp->cl->num * subsumer->fit / (double)subsumer->num;
        }
        subsumer->num += setp->cl->num;
        if(setpl==setp) {
          *aset=setp->next;
          deleteClassifierPointerFromSet(pop, setp->cl);
          freeClassifier(setp->cl);
          addClassifierToPointerSet(setp->cl, killset);
          free(setp);
          setp=*aset;
          setpl=*aset;
        }else{
          setpl->next=setp->next;
          deleteClassifierPointerFromSet(pop, setp->cl);
          freeClassifier(setp->cl);
          addClassifierToPointerSet(setp->cl, killset);
          free(setp);
          setp=setpl;
        }
      }
      setpl=setp;
    }
  }
}

/* Tries to subsume the parents */
void subsumeClassifier(struct xClassifier *cl, struct xClassifier **parents, struct xClassifierSet *locset, struct xClassifierSet **pop, int maxPopSize, int thetaSub, int doFitnessAdjustment)
{
  /*Try first to subsume in parents */
  int i;
  for(i=0; i<2; i++) {
    if(parents[i]!=NULL && subsumes(parents[i],cl, thetaSub)){
      if(doFitnessAdjustment)
        parents[i]->fit += parents[i]->fit / (double)parents[i]->num;
      parents[i]->num++;
      freeClassifier(cl);
      return;
    }
  }
  
  if(subsumeClassifierToSet(cl, locset, maxPopSize, thetaSub, doFitnessAdjustment))
    return;

  addClassifierToSet(cl, pop, doFitnessAdjustment);
}

/* Try to subsume in the specified set. */
int subsumeClassifierToSet(struct xClassifier *cl, struct xClassifierSet *set, int maxPopSize, int thetaSub, int doFitnessAdjustment)
{
  struct xClassifierSet * setp;
  struct xClassifier *subCl[maxPopSize];
  int numSub=0, spec=0;
  
  spec = getConditionLength();
  /*Try to subsume in the set*/
  for(setp=set; setp!=NULL; setp=setp->next) {
    if(subsumes(setp->cl, cl, thetaSub)) {
      if(spec > getSpecificity(setp->cl)) {
        spec = getSpecificity(setp->cl);
        numSub = 0;
      }
      subCl[numSub]=setp->cl;
      numSub++;
    }
  }
  /* if there were classifiers found to subsume, then choose randomly one and subsume */
  if(numSub>0) {
    numSub = (double)numSub * urand();
    if(doFitnessAdjustment)
      subCl[numSub]->fit += subCl[numSub]->fit / (double)subCl[numSub]->num;
    subCl[numSub]->num++;
    freeClassifier(cl);
    return 1;
  }
  return 0;
}

/* check if classifier cl1 subsumes cl2 */
int subsumes(struct xClassifier *cl1, struct xClassifier * cl2, int thetaSub)
{
  int ret=0;

  ret= cl1->act==cl2->act && isSubsumer(cl1, thetaSub) && isMoreGeneral( cl1->con, cl2->con);
 
  return ret;
}

/* returns if the classifier satisfies the criteria for being a subsumer */
int isSubsumer(struct xClassifier *cl, int thetaSub)
{
  return cl->num < MAX_SUBSUME_NUMEROSITY && cl->exp > thetaSub && cl->acc == 1.;
}

/* check if the first condition is more general than the second */
int isMoreGeneral (struct xCondition *first, struct xCondition *second)
{
  struct xCList *l1, *l2;
  int ret=0;

  for(l1=first->l, l2=second->l; l1!=NULL && l2!=NULL; l1=l1->next, l2=l2->next) {
    while(l2->pos < l1->pos) {
      ret = 1; /* definitely more specific */
      l2=l2->next;
      if(l2==NULL) /* since l1 still specifies we know that the first condition is not more general! */
        return 0;
    }
    if(l1->pos < l2->pos)
      return 0;
    /* if we came here, both pointers must point to the same position in the condition */
    if((l1->value).type == 0 && (l1->value).boole != (l2->value).boole)
      return 0;
    if( ((l1->value).type == 1 && ( (l1->value).lowerBound > (l2->value).lowerBound || (l1->value).upperBound < (l2->value).upperBound)) || 
	((l1->value).type == 2 && ( (l1->value).lowerRealBound > (l2->value).lowerRealBound || (l1->value).upperRealBound < (l2->value).upperRealBound)) 
	)
      return 0;
    if( ((l1->value).type == 1 && ( (l1->value).lowerBound < (l2->value).lowerBound || (l1->value).upperBound > (l2->value).upperBound)) || 
	((l1->value).type == 2 && ( (l1->value).lowerRealBound < (l2->value).lowerRealBound || (l1->value).upperRealBound > (l2->value).upperRealBound)) 
	)
      ret = 1;
  }
  if(l1!=NULL) /* since l1 still specifies we know that the first condition is not more general!*/
    return 0;
  if(l2!=NULL) /* l2 still specifies, thus, l1 is more general */
    ret = 1;

  return ret;
}


/*###################### adding classifiers to a set ###################################*/


/* adds only the pointers to the pointerset, ensures that no pointer is added twice, and
 * returns if the pointer was added */
int addClassifierToPointerSet(struct xClassifier *cl,struct xClassifierSet **pointerset)
{
  struct xClassifierSet *setp;
  
  for(setp=*pointerset; setp!=NULL; setp=setp->next) {
    if(setp->cl == cl){
      /* Classifier is already in Set */
      return 0;
    }
  }
  /* add the classifier, as it is not already in the pointerset */
  assert((setp=( struct xClassifierSet *)calloc(1,sizeof(struct xClassifierSet)))!=NULL);
  setp->cl=cl;
  setp->next=*pointerset;
  *pointerset=setp;
  return 1;
}

/* adds the classifier cl to the population, make sure that the same classifier does not exist, yet 
   returns if inserted classifier was deleted */
int addClassifierToSet(struct xClassifier *cl, struct xClassifierSet **clSet, int doFitnessAdjustment)
{
  struct xClassifierSet *setp;
  
  /* Check if classifier exists already. if so, just increase the numerosity and free the space of the new classifier */
  for(setp=*clSet;setp!=NULL;setp=setp->next){
    if(isEqualXC(setp->cl->con,cl->con) && setp->cl->act==cl->act) {
      if(doFitnessAdjustment) {
        setp->cl->fit += setp->cl->fit / (double)setp->cl->num;
      }
      setp->cl->num++;
      freeClassifier(cl);
      return 1;
    }
  }
  /* classifier does not exist, yet-> add new classifier */
  assert((setp=( struct xClassifierSet *)calloc(1,sizeof(struct xClassifierSet)))!=NULL);
  setp->cl=cl;
  setp->next=*clSet;
  *clSet=setp;
  return 0;
}

/* adds a new Clasifier to the xClassifierSet */
void addNewClassifierToSet(struct xClassifier *cl,struct xClassifierSet **clSet)
{
  struct xClassifierSet *setp;

  /* classifier does not exist, yet-> add new classifier */
  assert((setp=( struct xClassifierSet *)calloc(1,sizeof(struct xClassifierSet)))!=NULL);
  setp->cl=cl;
  setp->next=*clSet;
  *clSet=setp;
}


/*############################### deletion #############################################*/


/**
 * Deletes a classifier from the population and returns the pointer to the deleted classifier
 */
struct xClassifier * deleteClassifier(struct xClassifierSet **pop, struct XCS *xcs) {
  struct xClassifier *killedp = NULL;

  killedp=deleteStochClassifier(pop, xcs->delta, xcs->thetaDel, xcs->deletionType);

  return killedp;
}


struct xClassifier * deleteStochClassifier(struct xClassifierSet **pop, double delta, int thetaDel, int delType)
{
  struct xClassifierSet *setp,*setpl;
  struct xClassifier *killedp=NULL;
  double sum=0., choicep, meanf=0., meane=0.;
  int size=0;

  /* get the sum of the fitness and the numerosity */
  for(setp=*pop; setp!=NULL; setp=setp->next){
    meanf+=setp->cl->fit;
    meane+= setp->cl->preer*setp->cl->num;
    size+=setp->cl->num;
  }
  meanf/=(double)size;
  meane/=(double)size;

  /* get the delete proportion, which depends on the average fitness */
  for(setp=*pop; setp!=NULL; setp=setp->next) {
    sum += ((double)setp->cl->num * getDelPropMikro(setp->cl, meanf, meane, delta, thetaDel, delType));
  }

  /* choose the classifier that will be deleted */
  choicep=urand()*sum;
  
  /* look for the classifier */
  setp=*pop;
  setpl=*pop;
  sum= ((double)setp->cl->num * getDelPropMikro(setp->cl,meanf, meane, delta, thetaDel, delType));
  while(sum < choicep) {
    setpl=setp;
    setp=setp->next;
    sum += ((double)setp->cl->num * getDelPropMikro(setp->cl,meanf, meane, delta, thetaDel, delType));
  }
  
  /* delete the classifier */
  killedp=deleteTypeOfClassifier(setp, setpl, pop);
  
  /* return the pointer to the deleted classifier, to be able to update other sets */
  return killedp;
}

/* deletes the classifier setp from the population pop, setpl points to the classifier that is before setp in the list */
struct xClassifier * deleteTypeOfClassifier(struct xClassifierSet *setp,struct xClassifierSet *setpl,struct xClassifierSet **pop)
{
  struct xClassifier *killedp=NULL;

  /* setp must point to some classifier! */
  assert(setp!=NULL);

  if(setp->cl->num>1) {
    /* if the numerosity is greater than one -> just decrease it */
    setp->cl->num--;
    /*setp->cl->peerssest--;*/
  }else{
    /* if not, delete it and record it in killedp */
    if(setp==setpl) {
      *pop=setp->next;
    }else{
      setpl->next=setp->next;
    }
    killedp=setp->cl;
    freeClassifier(setp->cl);
    free(setp);
  }
  /* return a pointer ot a deleted classifier (NULL if the numerosity was just decreased) */
  return killedp;
}

/* return the delete proportion of one classifier, meanf is the average fitness in the population */
double getDelPropMikro(struct xClassifier *cl, double meanf, double meane, double delta, int thetaDel, int delType)
{
  double ret = 1;

  if((delType==1 || delType==2) && cl->exp >= thetaDel && (cl->fit/cl->num) < delta * meanf)
    ret *= meanf / (cl->fit / (double)cl->num);
  
  if(delType%2 == 1 ) {
    ret *= cl->peerssest;
  }
  
  if((delType==4 || delType == 5)) /* && cl-> exp >= thetaDel && cl->preer > meane)*/
    ret *= cl->preer/meane;
  
  return ret;
}


/* 
 * check if the classifier pointers that are in killset are in uset - delete the pointers if they are inside 
 * the classifiers in killset are already deleted, so do not read their values or try to delete them again
 */
int updateSet(struct xClassifierSet **uset,struct xClassifierSet *killset)
{
  struct xClassifierSet *setp,*setpl,*killp,*usetp;
  int updated=1;
  
  /* If one of the sets is empty -> do not do anything */
  if(*uset==NULL || killset==NULL)
    return 0;
  /* check all classifiers in uset */
  setp=*uset;
  while(updated && setp!=NULL) {
    setp=*uset;
    setpl=*uset;
    updated=0;
    while(setp!=NULL && !updated){
      for(killp=killset; killp!=NULL; killp=killp->next) {
        if(killp->cl == setp->cl){
          /* If killed classifier found, delete the struct classifier set in uset */
          updated=1;
          if(setp==setpl) {/* first entry in set */
            usetp=*uset;
            *uset=usetp->next;
            free(usetp);
            break;
          }else{
            setpl->next=setp->next;
            free(setp);
            setp = *uset;
            setpl= *uset;
            break;
          }
        }
      }
      /* check the whole uset again, if one pointer was deleted */
      if(updated)
        break;
      setpl=setp;
      setp=setp->next;
    }
  }
  /* return if the set was updated */
  return updated;
}

/* Deletes the classifier pointer from the specified set 
 * and returns if the pointer was found and deleted */
int deleteClassifierPointerFromSet(struct xClassifierSet **set, struct xClassifier *clp)
{
  struct xClassifierSet *setp, *setpl;
  for(setp=*set, setpl=*set; setp!=NULL; setp=setp->next){
    if(setp->cl==clp){
      if(setpl==setp){
        *set=(*set)->next;
        free(setp);
      }else{
        setpl->next=setp->next;
        free(setp);
      }
      return 1;
    }
    setpl=setp;
  }
  return 0;
}


/*############# concrete deletion of a classifier or a whole classifier set ############*/


/* Frees only the complete xClassifierSet (not the xClassifiers itself)! */
void freeSet(struct xClassifierSet **cls)
{
  struct xClassifierSet *clp;
  
  while(*cls!=NULL)
    {
      clp=(*cls)->next;
      free(*cls);
      *cls=clp;
    }
}

/* Frees the complete xClassifierSet with the corresponding xClassifiers */
void freeClassifierSet(struct xClassifierSet **cls)
{
  struct xClassifierSet *clp;

  while(*cls!=NULL)
    {
      freeClassifier((*cls)->cl);
      clp=(*cls)->next;
      free(*cls);
      *cls=clp;
    }
}

/* Frees one classifier */
void freeClassifier(struct xClassifier *cl)
{
  emptyXCondition(cl->con);
  free(cl->con);
  free(cl);
}


/*############################### output operations ####################################*/


/* print the classifiers in a xClassifierSet */
void printClassifierSet(struct xClassifierSet *head)
{
  for(;head!=NULL;head=head->next)
    printClassifier(head->cl);
}

/* print the classifier in a xClassifierSet to the file fp */
void fprintClassifierSet(FILE *fp,struct xClassifierSet *head)
{
  for(;head!=NULL;head=head->next)
    fprintClassifier(fp, head->cl);
}

/* print the classifier in a xClassifierSet to the file fp */
void fprintMicroClassifierSet(FILE *fp,struct xClassifierSet *head)
{
  int i;
  for(;head!=NULL;head=head->next) {
    for(i=0; i<head->cl->num; i++) {
      printXC(fp, head->cl->con);
    }
    fprintf(fp, "\n");
  }
}

/* print a single classifier */
void printClassifier(struct xClassifier *c)
{
  printXC(stdout, c->con);
  printf(" %d\t",c->act);  
  printf("%4.2f\t%4.2f\t%4.2f\t%4.2f\t", c->pre, c->preer, c->acc, c->fit);
  printf("%d %d %4.2f\t%d\n", c->num, c->exp, c->peerssest, c->gaIterationTime);
}

/* print a single classifier to the file fp */
void fprintClassifier(FILE *fp,struct xClassifier *c)
{
  /*  double realP=0, realMad=0;
   * char *cond;
   */
  
  printXC(fp, c->con);
  fprintf(fp," %d\t",c->act);
  /*
   * cond = getStringRepresentation(c->con);
   * realP = getExactEstimate(cond, c->act, &realMad);
   * free(cond);
   */
  fprintf(fp,"%f\t%f\t%f %f ", c->pre, c->preer, c->acc, c->fit);
  /*fprintf(fp,"%f=?%f\t%f=?%f\t%f %f ", c->pre, realP, c->preer, realMad, c->acc, c->fit);*/
  fprintf(fp,"%d %d %f %d\n", c->num, c->exp, c->peerssest, c->gaIterationTime);
}

struct xClassifierSet *readClassifierSet(FILE *fp)
{
  struct xClassifierSet *pop=NULL;
  struct xClassifier *nextClassifier;
  char cond[5000], s[5000];
  int cl,i,j;

  cl=getConditionLength();
  if(!fp) 
    return NULL;

  while(fscanf(fp,"%[^\n]\n",s)==1) {
    assert((nextClassifier = (struct xClassifier *)calloc(1,sizeof(struct xClassifier)))!=0);
    for(i=0; i<cl; i++) {
      cond[i]=s[i];
    }
    cond[i]='\0';
    nextClassifier->con = getConditionCode(cond);
    i++;
    /* get action code */
    for(j=0; s[i+j]!=' ' && s[i+j]!='\t'; j++)
      cond[j]=s[i+j];
    cond[j]='\0';
    nextClassifier->act = getIntValue(cond);
    i+=j+1;
    /* get prediction */
    for(j=0; s[i+j]!=' ' && s[i+j]!='\t'; j++)
      cond[j]=s[i+j];
    cond[j]='\0';
    nextClassifier->pre = getDoubleValue(cond);
    i+=j+1;
    /* get prediction error */
    for(j=0; s[i+j]!=' ' && s[i+j]!='\t'; j++)
      cond[j]=s[i+j];
    cond[j]='\0';
    nextClassifier->preer = getDoubleValue(cond);
    i+=j+1;
    /* get accuracy */
    for(j=0; s[i+j]!=' ' && s[i+j]!='\t'; j++)
      cond[j]=s[i+j];
    cond[j]='\0';
    nextClassifier->acc = getDoubleValue(cond);
    i+=j+1;
    /* get fitness */
    for(j=0; s[i+j]!=' ' && s[i+j]!='\t'; j++)
      cond[j]=s[i+j];
    cond[j]='\0';
    nextClassifier->fit = getDoubleValue(cond);
    i+=j+1;
    /* get numerosity */
    for(j=0; s[i+j]!=' ' && s[i+j]!='\t'; j++)
      cond[j]=s[i+j];
    cond[j]='\0';
    nextClassifier->num = getIntValue(cond);
    i+=j+1;
    /* get experience */
    for(j=0; s[i+j]!=' ' && s[i+j]!='\t'; j++)
      cond[j]=s[i+j];
    cond[j]='\0';
    nextClassifier->exp = getIntValue(cond);
    i+=j+1;
    /* get peerssest */
    for(j=0; s[i+j]!=' ' && s[i+j]!='\t'; j++)
      cond[j]=s[i+j];
    cond[j]='\0';
    nextClassifier->peerssest = getDoubleValue(cond);
    i+=j+1;
    /* get gaTimeStamp*/
    for(j=0; s[i+j]!=' ' && s[i+j]!='\t' && s[i+j]!='\0'; j++)
      cond[j]=s[i+j];
    cond[j]='\0';
    nextClassifier->gaIterationTime = getIntValue(cond);
    i+=j+1;
    /* insert next classifier */
    addNewClassifierToSet(nextClassifier, &pop);
    /*printf("Added: ");printClassifier(nextClassifier);*/
  }
  return pop;
}

/*###################### sorting and classifier list statistics ########################*/


/* sort the classifier set cls in numerosity order or in prdiction order 
 * type 0 = numerosity order, type 1 = prediction order, type 2 = fitness order, type 3=error order */
struct xClassifierSet * sortClassifierSet(struct xClassifierSet **cls,int type)
{
  struct xClassifierSet *clsp, *maxcl, *newcls, *newclshead;
  double max;

  max=0.;
  assert((newclshead=( struct xClassifierSet *)calloc(1,sizeof(struct xClassifierSet)))!=NULL);
  newcls=newclshead;
  do{
    max=-100000;
    /* check the classifier set cls for the next maximum -> already inserted classifier are referenced by the NULL pointer */
    for( clsp=*cls, maxcl=NULL; clsp!=NULL; clsp=clsp->next ) {
      if(clsp->cl!=NULL && (maxcl==NULL || ((type==0 && clsp->cl->num>max) || (type==1 && clsp->cl->pre>max) || (type==2 && clsp->cl->fit/clsp->cl->num > max) || (type==3 && -1.*(clsp->cl->preer) > max)))) {
        if(type==0)
          max=clsp->cl->num;
        else if (type==1)
          max=clsp->cl->pre;
        else if (type==2)
          max=clsp->cl->fit/clsp->cl->num;
        else if(type==3)
          max=-1.*(clsp->cl->preer);
        maxcl=clsp;
      }
    }
    if(max>-100000) {
      assert((newcls->next=( struct xClassifierSet *)calloc(1,sizeof(struct xClassifierSet)))!=NULL);
      newcls=newcls->next;
      newcls->next=NULL;
      newcls->cl=maxcl->cl;
      /* do not delete the classifier itself, as it will be present in the new, sorted classifier list */
      maxcl->cl=NULL;
    }
  }while(max>-100000);
  
  /* set the new xClassifierSet pointer and free the old stuff */
  newcls=newclshead->next;
  free(newclshead);
  freeSet(cls);
  /* return the pointer to the new xClassifierSet */
  return newcls;
}


/* Filters the classifier set removing all classifiers that have a low prediction,
 * a low experience, and/or a high error 
 * Does not destroy the old classifier set.
 * Returns a new pointer set that points to the old classifiers.
 */
struct xClassifierSet * filterClassifierSet(struct xClassifierSet *cls, double minPre, int minExp, int minNum, double maxError)
{
  struct xClassifierSet *clsp, *newclsp, *newclshead;

  assert((newclshead=( struct xClassifierSet *)calloc(1,sizeof(struct xClassifierSet)))!=NULL);
  newclsp=newclshead;
  for(clsp = cls; clsp!=NULL; clsp=clsp->next) {
    if( ((clsp->cl->pre >= minPre) || ( clsp->cl->pre < minPre && getNumberOfActions()==2)) &&
       clsp->cl->exp >= minExp && clsp->cl->num >= minNum && clsp->cl->preer <= maxError) {
      /* classifier OK -> add it to the new list */
      assert((newclsp->next=( struct xClassifierSet *)calloc(1,sizeof(struct xClassifierSet)))!=NULL);
      newclsp=newclsp->next;
      newclsp->next=NULL;
      newclsp->cl = clsp->cl;
      if(newclsp->cl->pre < minPre && getNumberOfActions()==2) {
	newclsp->cl->pre = getPaymentRange() - newclsp->cl->pre;
	newclsp->cl->act = (newclsp->cl->act + 1)%2; 
      }
    }
  }
  
  /* set the new xClassifierSet pointer and free the old stuff */
  newclsp=newclshead->next;
  free(newclshead);
  /* return the pointer to the new xClassifierSet */
  return newclsp;
}


/**
 * Calculates the specificity of each bit in the conditions of the population.
 * The array specPos is assumed to have allocated the conditionLength size!
 */
void getPosSpecificity(struct xClassifierSet *cll, double *specPos)
{
  struct xCList *l;
  int i;

  for(i=0; i<getConditionLength(); i++) {
    specPos[i]=0;
  }
  for( ; cll!=NULL && cll->cl!=NULL; cll=cll->next) {
    for(l = cll->cl->con->l; l!=NULL; l=l->next) {
      switch((l->value).type) {
      case 0:
	specPos[l->pos] += cll->cl->num;
	break;
      case 1:
	specPos[l->pos] += (1. - ((l->value).upperBound - (l->value).lowerBound)/(getUpperBound(l->pos) - getLowerBound(l->pos))) * cll->cl->num;
	break;
      case 2:
	specPos[l->pos] += (1. - ((l->value).upperRealBound - (l->value).lowerRealBound)/(getUpperBound(l->pos) - getLowerBound(l->pos))) * cll->cl->num;
	break;
      default:
	printf("Specificity not determinable for this attribute type!\n");
	break;
      }
    }
  }
}

/**
 * Determines specificity and standard deviation of the specificity in the given set.
 */
void getSpecAndSDev(struct xClassifierSet *set, double *spec, double *specSDev) 
{
  struct xClassifierSet *setp;
  double length, size;

  *spec=0.;
  length = (double)getConditionLength();
  size=0;
  for(setp=set; setp!=NULL; setp=setp->next) {
    (*spec) += (double)setp->cl->num * (double)getSpecificity(setp->cl) / length;
    size += (double)setp->cl->num;
  }
  *specSDev = 0.;
  if(size!=0) {
    (*spec) /= size;
    for(setp=set; setp!=NULL; setp=setp->next) {
      (*specSDev) += (double)setp->cl->num * (((double)getSpecificity(setp->cl) / length) - *spec) * (((double)getSpecificity(setp->cl) / length) - *spec);
    }
    *specSDev = sqrt((*specSDev)/size);
  }
}


/*############################### Operation on Condition Part ##########################*/

int addXCBounds(struct xCondition *con, int pos, int low, int up) 
{
  struct xCList *l, *ll=NULL;
  
  for(l=con->l; l!=NULL; l=l->next) {
    if(l->pos >= pos) {
      if(l->pos == pos)
        return 0; /* position is already specified */
      else 
        break; /* position is further than the one to be inserted */
    }
    ll=l;
  }
  /* position will be added */
  con->size ++;
  if(ll==NULL) { /* add before first one */
    assert((con->l = (struct xCList *)calloc(1,sizeof(struct xCList)))!=0);
    con->l->next = l;
    l=con->l;
  }else{ /* add one more */
    assert((ll->next = (struct xCList *)calloc(1,sizeof(struct xCList)))!=0);
    ll->next->next=l;
    l=ll->next;
  }
  l->pos = pos;
  (l->value).type=1;
  (l->value).lowerBound = low;
  (l->value).upperBound = up;
  return 1;
}

int addXCRealBounds(struct xCondition *con, int pos, double low, double up) 
{
  struct xCList *l, *ll=NULL;
  
  for(l=con->l; l!=NULL; l=l->next) {
    if(l->pos >= pos) {
      if(l->pos == pos)
        return 0; /* position is already specified */
      else 
        break; /* position is further than the one to be inserted */
    }
    ll=l;
  }
  /* position will be added */
  con->size ++;
  if(ll==NULL) { /* add before first one */
    assert((con->l = (struct xCList *)calloc(1,sizeof(struct xCList)))!=0);
    con->l->next = l;
    l=con->l;
  }else{ /* add one more */
    assert((ll->next = (struct xCList *)calloc(1,sizeof(struct xCList)))!=0);
    ll->next->next=l;
    l=ll->next;
  }
  l->pos = pos;
  (l->value).type=2;
  (l->value).lowerRealBound = low;
  (l->value).upperRealBound = up;
  return 1;
}

/* adds specified position 'c' add position pos to condition con */
int addXCPos(struct xCondition *con, int pos, char c)
{
  struct xCList *l, *ll=NULL;

  for(l=con->l; l!=NULL; l=l->next) {
    if(l->pos>=pos) {
      if(l->pos == pos)
        return 0; /* position is already specified */
      else 
        break; /* position is further than the one to be inserted */
    }
    ll=l;
  }
  /* position will be added */
  con->size ++;
  if(ll==NULL) { /* add before first one */
    assert((con->l = (struct xCList *)calloc(1,sizeof(struct xCList)))!=0);
    con->l->next = l;
    l=con->l;
  }else{ /* add one more */
    assert((ll->next = (struct xCList *)calloc(1,sizeof(struct xCList)))!=0);
    ll->next->next=l;
    l=ll->next;
  }
  l->pos = pos;
  (l->value).type = 0;
  (l->value).boole = c;
  return 1;
}

/* copies the condition */
struct xCondition * copyXC(struct xCondition *cOld)
{
  struct xCList *cl, *clnew;
  struct xCondition *cNew;

  assert((cNew=(struct xCondition *)calloc(1,sizeof(struct xCondition)))!=NULL);
  cNew->l=NULL;
  cNew->size=0;

  clnew = NULL;
  for(cl=cOld->l; cl!=NULL; cl=cl->next) {
    if(clnew==NULL) { /* add first element in new condition */
      assert((cNew->l = (struct xCList *)calloc(1,sizeof(struct xCList)))!=0);
      clnew = cNew->l;
    }else{ /* add next element in new condition */
      assert((clnew->next = (struct xCList *)calloc(1,sizeof(struct xCList)))!=0);
      clnew = clnew->next;
    }
    /* copy from the old classifier */
    clnew->pos = cl->pos;
    clnew->value = cl->value;
  }
  cNew->size = cOld->size;

  return cNew;
}

/* checks for equality */
int isEqualXC(struct xCondition *c1, struct xCondition *c2)
{
  struct xCList *l1, *l2;

  for(l1=c1->l, l2=c2->l; l1!=NULL && l2!=NULL; l1=l1->next, l2=l2->next) {
    if(l1->pos != l2->pos || (l1->value).type != (l2->value).type || (
         ( (l1->value).type==0 && (l1->value).boole != (l2->value).boole ) || 
           ((l1->value).type==1 && 
            ((l1->value).lowerBound != (l2->value).lowerBound || (l1->value).upperBound != (l2->value).upperBound)) ||
           ((l1->value).type==2 && 
            ((l1->value).lowerRealBound != (l2->value).lowerRealBound || (l1->value).upperRealBound != (l2->value).upperRealBound))
         )
       )
      return 0;
  }
  if(l1==NULL && l2==NULL)
    return 1;
  return 0;
}

/* prints the condition in standard form */
void printXC(FILE *fp, struct xCondition *con)
{
  struct xCList *l, *lh;
  int i=0;
  int length;

  for(l=con->l; l!=NULL; l=l->next) {
    for( ; i<l->pos; i++)
      fprintf(fp, "#");
    fprintCondAttribute(fp, l->value);
    for(lh = l->next; lh!=NULL; lh=lh->next) {
      if(lh->pos <= l->pos)
        fprintf(fp, "MISTAKE - DOUBLE POS SPECIFICATION");
    }
    i++;
  }
  length = getConditionLength();
  for( ; i<length; i++)
    fprintf(fp, "#");
}

/* generates the corresponding condition out of the character coded condition */
struct xCondition *getConditionCode(char *condition)
{
  int i;
  struct xCondition *con;
  struct xCList *l;
  
  if(getGeneralType()==0) {
    assert((con=(struct xCondition *)calloc(1, sizeof(struct xCondition)))!=0);
    for(i=0; i<getConditionLength(); i++) {
      if(condition[i]!='#') {
        /* add specified attribute */
        if(con->l==0) {
          assert((con->l = (struct xCList *)calloc(1, sizeof(struct xCList)))!=0);
          l=con->l;
        }else{
          assert((l->next = (struct xCList *)calloc(1, sizeof(struct xCList)))!=0);
          l=l->next;
        }
        (l->value).type=0;
        (l->value).boole = condition[i];
        l->pos=i;
        con->size++;
      }
    }
  }else{
    printf("Other types are not supported for conversion from string representation!!!\n");
    return NULL;
  }
  return con;
}

/**
 * return a string rerpresenting the condition 
 * only the character-based boolean representation is supported here!!! 
 */
char *getStringRepresentation(struct xCondition *con)
{
  int i, length;
  char *res;
  struct xCList *l;

  if(getGeneralType() == 0) {
    length = getConditionLength();
    assert((res=(char*)calloc(length+1,sizeof(char)))!=NULL);
    
    for(i=0, l=con->l; l!=NULL; l=l->next) {
      for( ; i<l->pos; i++)
        res[i]='#';
      res[i]= (l->value).boole;
      i++;
    }
    for( ; i<length; i++)
      res[i]='#';
    res[i]='\0';
    return res;
  }
  return NULL;
}

/**
 * Returns 1 if this condition attribute matches the current attribute in the given problem instance
 */ 
int doesMatchAttribute(struct condAttribute cond, struct attributes att)
{
  switch(att.type) { /* using the type in state to take care of don't care cases */
    case 0: /* Boolean attribute */
      if( cond.boole != att.boole && att.boole !=DONT_CARE && cond.boole != DONT_CARE)
        return 0;
      break;
    case 1: /* Integer attribute */
      if( cond.lowerBound > att.integer || cond.upperBound < att.integer)
        return 0;
      break;
    case 2: /* Real attribute */
      if( cond.lowerRealBound > att.real || cond.upperRealBound < att.real)
        return 0;
      break;
  default: /* not supported */
    printf("This format is not supported, yet!!!\n");
    fprintCondAttribute(stdout, cond);
    fprintAttributes(stdout, att);
    fprintf(stdout,"\n");
    break;
  }
  return 1;
}

/**
 * Returns the condition attribute at the specified position.
 * Works only for the boolean CASE!!!
 */
struct condAttribute *getAttribute(struct xCondition *con, int pos) 
{
  struct xCList *l;
  struct condAttribute *res=NULL;

  assert((res=(struct condAttribute *)calloc(1,sizeof(struct condAttribute)))!=NULL);

  for(l = con->l; l!=NULL; l=l->next) {
    if(l->pos >=pos) {
      if( l->pos > pos) {
	res->type = 0;
	res->boole = DONT_CARE;
      }else{
	res->type = (l->value).type;
	res->boole = (l->value).boole;
	res->lowerBound = (l->value).lowerBound;
	res->upperBound = (l->value).upperBound;
	res->lowerRealBound = (l->value).lowerRealBound;
	res->upperRealBound = (l->value).upperRealBound;
      }
      return res;
    }
  }
  res->type = getAttributeType(pos);
  switch(res->type){
  case 0:
    res->boole = DONT_CARE; 
    break;
  case 1: 
    res->lowerBound = (int)getLowerBound(pos); 
    res->upperBound = (int)getUpperBound(pos); 
    break;
  case 2:
    res->lowerRealBound = getLowerBound(pos); 
    res->upperRealBound = getUpperBound(pos); 
    break;
  default:
    printf("Case Not Covered in getAttribute(struct xCondition *con, int pos)\n");
    res->boole = DONT_CARE; 
    break;
  }    
  return res;
}

/**
 * return an array with all attributes in condAttribute format
 */
struct condAttribute *getAttributeArray(struct xCondition *con)
{
  int i, length;
  struct condAttribute *res=NULL;
  struct xCList *l;

  length = getConditionLength();
  assert((res=(struct condAttribute *)calloc(length,sizeof(struct condAttribute)))!=NULL);
    
  for(i=0, l=con->l; i<length; i++) {
    for( ; (l==NULL && i<length) || (l!=NULL && l->pos>i); i++) {
      res[i].type = getAttributeType(i);
      switch(res[i].type) {
      case 0:
	res[i].boole = DONT_CARE;
	break;
      case 1:
	res[i].lowerBound = getLowerBound(i);
	res[i].upperBound = getUpperBound(i);
	break;
      case 2:
	res[i].lowerRealBound = getLowerBound(i);
	res[i].upperRealBound = getUpperBound(i);
	break;
      default:
	printf("Type %d not supported\n", res[i].type);
	break;
      }
    }
    if(l!=NULL) {
      res[i].type = (l->value).type;
      res[i].boole = (l->value).boole;
      res[i].lowerBound = (l->value).lowerBound;
      res[i].upperBound = (l->value).upperBound;
      res[i].lowerRealBound = (l->value).lowerRealBound;
      res[i].upperRealBound = (l->value).upperRealBound;
      l=l->next;
    }
  }
  return res;
}

/* empties the condition and sets its length to zero */
void emptyXCondition(struct xCondition *con)
{
  struct xCList *cl, *clHelp;
  for(cl=con->l; cl!=NULL; ) {
    clHelp=cl;
    cl=cl->next;
    clHelp->next=NULL;
    free(clHelp);
  }
  con->l = NULL;
}

/* resets the specificity size parameter of the condition */
void resetXCSize(struct xCondition *con)
{
  int i=0;
  struct xCList *l;
  for(l=con->l; l!=NULL; l=l->next)
    i++;
  con->size = i;
}

void copyAttributes(struct attributes *to, struct attributes *from)
{
  int i;

  for(i=0; i<getConditionLength(); i++) {
    to[i].type = from[i].type;
    to[i].boole = from[i].boole;
    to[i].integer = from[i].integer;
    to[i].real = from[i].real;
  }
}

void fprintState(FILE *fp, struct attributes *state)
{
  int i;

  for(i=0; i<getConditionLength(); i++) {
    switch(state[i].type) {
    case 0: fprintf(fp, "%c", state[i].boole); break;
    case 1: fprintf(fp, "%d", state[i].integer); break;
    case 2: fprintf(fp, "%4.2f", state[i].real); break;
    default: fprintf(fp, "?"); break;
    }
  }
}

void fprintCondAttribute(FILE *fp, struct condAttribute att)
{
  switch(att.type) {
  case 0: fprintf(fp, "%c", att.boole); break;
  case 1: fprintf(fp, "[%d,%d]", att.lowerBound, att.upperBound); break;
  case 2: fprintf(fp, "[%4.2f,%4.2f]", att.lowerRealBound, att.upperRealBound); break;
  default: fprintf(fp, "?"); break;
  }
}

void fprintAttributes(FILE *fp, struct attributes att)
{
  switch(att.type) {
  case 0: fprintf(fp, "%c", att.boole); break;
  case 1: fprintf(fp, "%d", att.integer); break;
  case 2: fprintf(fp, "%4.2f", att.real); break;
  default: fprintf(fp, "?"); break;
  }
}
