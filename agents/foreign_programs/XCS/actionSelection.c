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
/     Everything that is related to the action selection procedure.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "classifierList.h"
#include "actionSelection.h"
#include "xcsMacros.h"
#include "env.h"

/* Determines the prediction array out of the classifier set ms */
void getPredictionArray(struct xClassifierSet *ms, double *pa, struct XCS *xcs)
{
  double *fnr, fit;
  int i, act;  

  assert((fnr = (double *)calloc(getNumberOfActions(),sizeof(double)))!=0);
  /* ms should never be NULL (because of covering) */
  assert(ms!=NULL);
  
  /* set the values to 0 (fnr memorizes the sum of the fitnesses) */  
  for(i=0;i<getNumberOfActions();i++) {
    pa[i]=0;
    fnr[i]=0;
  }

  /* Get the sum of the fitnesses and the predictions */
  for( ; ms!=NULL ; ms=ms->next) {
    act=ms->cl->act;
    fit=ms->cl->fit;
    pa[act] += (ms->cl->pre*fit);
    fnr[act]+= fit;
  }

  /* Divide the sum of the predictions by the sum of the fitnesses */
  for(i=0 ; i<getNumberOfActions() ;i++){
    if(fnr[i] != 0){
      pa[i]/=fnr[i];
      if(pa[i]==0) {
	/* Changed zero entry in prediction array to prevent infinite loops 05/25/2000 */
	pa[i]=0.0000001;
      }
    }else{
      pa[i]=0;
    }
  }
  free(fnr);
}

/* choose the apropriate action */
int learningActionWinner(double *predictionArray, double exploreProb)
{
  if(urand() < exploreProb) 
    return randomActionWinner(predictionArray);
  else
    return deterministicActionWinner(predictionArray);
}

/* choose randomly one of the possiblactions */
int randomActionWinner(double *predictionArray)
{
  return (int)(urand()*(double)getNumberOfActions());
}

/* choose the best action in the prediction array */
int deterministicActionWinner(double *predictionArray)
{
  int i,ret=0;
  double max=0.;
  
  for(i=0;i<getNumberOfActions();i++){
    if(predictionArray[i]>max){
      max=predictionArray[i];
      ret=i;
    }
  }
  return ret;
}

/* choose the action by using the roulette wheel */
int rouletteWheelActionWinner(double *predictionArray)
{
  double sum=0.,sel;
  int i;
  
  for(i=0;i<getNumberOfActions();i++)
    sum+=predictionArray[i];
  
  sel=urand()*sum;
  sum=predictionArray[0];
  i=0;
  while(sel>sum){
    i++;
    sum+=predictionArray[i];
  }
  return i;
}

