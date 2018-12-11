#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "treeRepresentation.h"

struct Tree *getTreeRecursive(struct xClassifierSet *cll, double *specs, int *used, double *predArray, struct XCS *xcs, int depth);

/**
 * The "public" tree generation function. 
 */
struct Tree* generateTree(struct xClassifierSet *pop, struct XCS *xcs)
{
  double *specs;
  int *used;
  struct Tree* ret;
  double *predictionArray;
  struct xClassifierSet *cll;

  assert((specs = (double *)calloc(getConditionLength(), sizeof(double)))!=0);
  assert((used = (int *)calloc(getConditionLength(), sizeof(int)))!=0);
  assert((predictionArray = (double *)calloc(getNumberOfActions(),sizeof(double)))!=0);

  cll = copyClassifierSet(pop);
  ret = getTreeRecursive(cll, specs, used, predictionArray, xcs, 0);

  free(predictionArray);
  free(used);
  free(specs);
  return ret;
}

/**
 * Generates the tree nodes recursively.
 */
struct Tree *getTreeRecursive(struct xClassifierSet *cll, double *specs, int *used, double *predArray, struct XCS *xcs, int depth)
{
  int i, length, pos;
  double max;
  struct xClassifierSet **subSets;
  struct condAttribute *conAtt;
  struct Tree *retTree;

  assert((retTree = (struct Tree *)calloc(1, sizeof(struct Tree)))!=NULL);
  retTree->representatives = cll;
  retTree->support = getNumerositySum(cll);
  retTree->depth = depth;

  /* printf("Recursion %d!", depth); */

  /* determine the predicted class of this node in the tree */
  getPredictionArray(cll, predArray, xcs);
  max=0;
  pos=-1;
  for(i=0; i<getNumberOfActions(); i++) {
    if(pos==-1 || max < predArray[i]) {
      max = predArray[i];
      pos = i;
    }
  }
  retTree->class_index = pos;
  
  /* determine the next position to split on */
  getPosSpecificity(cll, specs);
  pos=-1;
  max=0;
  length = getConditionLength();
  for(i=0; i<length; i++) {
    /* printf("%1.3f ", specs[i]);*/
    if(used[i]==0) {
      if(pos==-1 || max < specs[i]) {
	pos = i;
	max = specs[i];
      }
    }
  }
  if(pos==-1 || max==0) { /* nothing to split anymore */
    /*printf("NULL\n");*/
    return retTree;
  }
  /* pos is the one with the highest specificity */
  retTree->attPos = pos;
  used[pos] = 1;
  /* printf("Splitting on Node %d...\n",pos);*/
  
  /* partition the population */
  if(getAttributeType(pos)>1 || (getAttributeType(pos)==1 && getUpperBound(pos)-getLowerBound(pos)>1)) {
    printf("Do not support real values or integers with more than two values!!!\n");
    return retTree;
  }
  
  assert((subSets = (struct xClassifierSet **)calloc(3, sizeof(struct xClassifierSet *)))!=NULL);

  for( ; cll!=NULL && cll->cl!=NULL ; cll=cll->next) {
    conAtt = getAttribute(cll->cl->con, pos);
    if( (conAtt->type==0 && conAtt->boole == '0') || 
        (conAtt->type==1 && conAtt->upperBound == (int)getLowerBound(pos)) 
       ) {
      addNewClassifierToSet(cll->cl, &subSets[0]);
    }else if( (conAtt->type==0 && conAtt->boole == '1') ||
	      (conAtt->type==1 && conAtt->lowerBound == (int)getUpperBound(pos)) 
	      ) {
      addNewClassifierToSet(cll->cl, &subSets[1]);
    }else{
      addNewClassifierToSet(cll->cl, &subSets[2]);
    }
  }
  
  retTree->numChildren = 0;
  assert((retTree->children = (struct Tree **)calloc(3, sizeof(struct Tree*)))!=NULL);
  if(subSets[0]!=NULL) {
    /*
     * printf("(%d)%d=0:", depth, retTree->attPos);
     * printClassifierSet(zeroL);
     */
    (retTree->children)[0] = getTreeRecursive(subSets[0], specs, used, predArray, xcs, depth+1);
    (retTree->numChildren)++;
  }
  if(subSets[1]!=NULL) {
    /* 
     * printf("(%d)%d=1:", depth, retTree->attPos);
     * printClassifierSet(oneL);
     */
    (retTree->children)[1] = getTreeRecursive(subSets[1], specs, used, predArray, xcs, depth+1);
    (retTree->numChildren)++;
  }
  if(subSets[2]!=NULL) {
    /* 
     * printf("(%d)%d=#:", depth, retTree->attPos);
     * printClassifierSet(hashL);
     */
    (retTree->children)[2] = getTreeRecursive(subSets[2], specs, used, predArray, xcs, depth+1);
    (retTree->numChildren)++;
  }
  used[retTree->attPos]=0;
  free(subSets);
  return retTree;
}


/**
 * Deletes all allocated memory for this tree
 */
void deleteTree(struct Tree **tree)
{
  struct Tree *thisTree = *tree;
  int i;

  if(thisTree->children!=NULL) {
    for(i=0; i<3; i++) {
      if((thisTree->children)[i]!=NULL)
	deleteTree(&((thisTree->children)[i]));
    }
    free(thisTree->children);
  }
  freeSet(&(thisTree->representatives));
  free(*tree);
}

void deleteForest(struct Tree ***forest)
{
  int i;
  
  for(i=0; i<getConditionLength(); i++) {
    if((*forest)[i]!=NULL) {
      deleteTree( &((*forest)[i]));
    }
  }
  free(*forest);
}

/**
 * Evaluates the classification performance of the tree.
 */
double testTree(struct Tree *tree)
{
  struct attributes *state;
  int correct, class, correctSum, tests;

  printf("Testing Tree...\n");
  assert((state=(struct attributes *)(calloc(getConditionLength(),sizeof(struct attributes))))!=0);
  correct=0;
  correctSum=0;
  
  doTesting(1);
  tests=0;
  while(nextTestState(state)) {
    tests++;
    
    /*fprintState(stdout, state);*/
    class = getClassification(tree, state);

    doAction(state, class, &correct);
    if(correct == 0) {
      /*printf(" != %d!?\n", class);*/
    }
    correctSum += correct;
  }
  free(state);
  printf(" Tree Classified %d out of %d correct!\n", correctSum, tests);
  return (double)correctSum/(double)tests;
}

/**
 * Returns the class proposed by the tree structure.
 * Note that underrepresented branches are not descendet.
 * Current algorithm is linear in the tree depth. 
 * If an attribute is underrepresented, the hash-branch is descendet.
 */
int getClassification(struct Tree *tree, struct attributes *probInstance)
{
  int length;
  int child, otherChild, matchSupport, matchSupport2;

  length = getConditionLength();

  /* if not children or not enought support for splitting return this class */
  if(tree->numChildren==0 || tree->support<5 || tree->children == NULL) {
    /*printf("->%d\n:", tree->class_index);*/
    return tree->class_index;
  }

  /* determine which is the child */
  if( (probInstance[tree->attPos].type==0 && probInstance[tree->attPos].boole == '0') ||
      (probInstance[tree->attPos].type==1 && probInstance[tree->attPos].integer == (int)getLowerBound(tree->attPos))
      ) {
    child=0;
    otherChild=1;
  }else{
    child=1;
    otherChild=0;
  }
  matchSupport = 0;
  matchSupport2 = 0;
  /* get matching support of both available paths */
  if( tree->children[child] != NULL)
    matchSupport = getMatchSupport(probInstance, tree->children[child]->representatives);
  if( tree->children[2] != NULL)
    matchSupport2 = getMatchSupport(probInstance, tree->children[2]->representatives);
  
  if(matchSupport>0 || matchSupport2>0) {
    if(matchSupport >= matchSupport2) {
      /*printf("Down %d=%d", ((tree->attPos)+1), child);*/
      return getClassification(tree->children[child], probInstance);
    }else{
      /*printf("descending don't care path...");*/
      /*printf("Down %d=#", ((tree->attPos)+1));*/
      return getClassification(tree->children[2], probInstance);
    }
  }
  /*printf("-0->%d\n", tree->class_index);*/
  return tree->class_index;
}




void fprintAllTreeRecursive(FILE *fp, struct Tree *tree, int indent, int type, struct XCS *xcs, double *predArray);

/**
 * Prints hte complete tree with all the information in it.
 */
void fprintAllTree(FILE *fp, struct Tree *tree, int type, struct XCS *xcs)
{
  double *predictionArray;
  assert((predictionArray = (double *)calloc(getNumberOfActions(),sizeof(double)))!=0);
  fprintAllTreeRecursive(fp, tree, 0, type, xcs, predictionArray);
  free(predictionArray);
}

void fprintAllTreeRecursive(FILE *fp, struct Tree *tree, int indent, int type, struct XCS *xcs, double *predArray)
{
  int i;
  
  for(i=0; i<indent; i++)
    fprintf(fp, " -");
  getPredictionArray(tree->representatives, predArray, xcs);
  fprintf(fp, "Nod.:%d Sup.:%d Cla.:%d (", tree->attPos, tree->support, tree->class_index);
  for(i=0; i<getNumberOfActions(); i++)
    fprintf(fp, "%f=s:%d, ", predArray[i], getActionSupport(i, tree->representatives));
  fprintf(fp, ") Chi.Sup. ");
  for(i=0; i<3; i++) {
    if(tree->children != NULL && (tree->children)[i] != NULL)
      fprintf(fp, "%d-", ((tree->children)[i])->support);
    else
      fprintf(fp, "00-");
  }
  fprintf(fp, "\n");
  indent++;
  if(indent<8) {
    for(i=0; i<2; i++) {
      if(tree->children != NULL && (tree->children)[i] != NULL && ((tree->children)[i])->support>0) {
	fprintf(fp, "%d:\n", i); 
	fprintAllTreeRecursive(fp, (tree->children)[i], indent, type, xcs, predArray);
      }
      /*printf("%d**************\n", i);*/
    }
  }
}


void fprintDOTTreeRecursive(FILE *fp, struct Tree *tree, char *ID, char *ID2, int type);

/**
 * Prints the tree to be converted in a visualizable tree in the dot format.
 */
void fprintDOTTree(FILE *fp, struct Tree *tree, int type) 
{
  char *ID, *ID2;
  int j;

  assert((ID = (char*)calloc(getConditionLength()+3, sizeof(char)))!=NULL);
  assert((ID2 = (char*)calloc(getConditionLength()+3, sizeof(char)))!=NULL);

  for(j=0; j<getConditionLength(); j++) {
    ID[j]='#';
    ID2[j]='#';
  }
  ID[j]='-';
  ID2[j]='-';
  ID[j+1]='?';
  ID2[j+1]='?';
  
  fprintf(fp, "digraph G {\n");
  fprintf(fp, "rotate=90;\n");
  fprintf(fp, "size=\"7,7\";\n");

  fprintDOTTreeRecursive(fp, tree, ID, ID2, type);

  fprintf(fp, "}\n");
  free(ID);
  free(ID2);
}

void fprintDOTTreeRecursive(FILE *fp, struct Tree *tree, char *ID, char *ID2, int type)
{
  ID[getConditionLength()+1]=(char)((int)'0'+(tree->class_index));
  if(tree->numChildren == 0 ) {
    fprintf(fp, "\"%s\" [label = \"%c\"]", ID, ID[getConditionLength()+1]);
  }else{
    fprintf(fp, "\"%s\" [shape=record,label = \"{%d=%c|{<0>0|<1>1|<#>#}}\"]", ID, (tree->attPos)+1, ID[getConditionLength()+1]);
    if(tree->children != NULL && (tree->children)[0] != NULL) {
      ID2[getConditionLength()+1]=(char)((int)'0'+((tree->children)[0])->class_index);
      ID2[tree->attPos] = '0';
      fprintf(fp, "\"%s\":\"0\" -> \"%s\";\n", ID, ID2); 
      ID[tree->attPos] = '0';
      fprintDOTTreeRecursive(fp, (tree->children)[0], ID, ID2, type);
      ID[getConditionLength()+1]=(char)((int)'0'+(tree->class_index));
      ID[tree->attPos] = '#';
      ID2[tree->attPos] = '#';
    }
    if(tree->children != NULL && (tree->children)[1] != NULL) {
      ID2[getConditionLength()+1]=(char)((int)'0'+((tree->children)[1])->class_index);
      ID2[tree->attPos] = '1';
      fprintf(fp, "\"%s\":\"1\" -> \"%s\";\n", ID, ID2); 
      ID[tree->attPos] = '1';
      fprintDOTTreeRecursive(fp, (tree->children)[1], ID, ID2, type);
      ID[getConditionLength()+1]=(char)((int)'0'+(tree->class_index));
      ID[tree->attPos] = '#';
      ID2[tree->attPos] = '#';
    }
    if((type)  && tree->children != NULL && (tree->children)[2] != NULL) {
      ID2[getConditionLength()+1]=(char)((int)'0'+((tree->children)[2])->class_index);
      ID2[tree->attPos] = '*';
      fprintf(fp, "\"%s\":\"#\" -> \"%s\";\n", ID, ID2); 
      ID[tree->attPos] = '*';
      fprintDOTTreeRecursive(fp, (tree->children)[2], ID, ID2, type);
      ID[getConditionLength()+1]=(char)((int)'0'+(tree->class_index));
      ID[tree->attPos] = '#';
      ID2[tree->attPos] = '#';
    }
  }
}
























void fprintAllForest(FILE *fp, struct Tree **forest, int type, struct XCS *xcs)
{
  int i;
  for(i=0; i<getConditionLength(); i++){
    if(forest[i]!=NULL)
      fprintAllTree(fp, forest[i], type, xcs);
  }
}


void fprintDOTForest(FILE *fp, struct Tree **forest, int type)
{
  int i;
  for(i=0; i<getConditionLength(); i++){
    if(forest[i]!=NULL)
      fprintDOTTree(fp, forest[i], type);
  }
}



struct Tree *getForestRecursive(struct xClassifierSet *cll, double *specs, int *used, double *predArray, struct XCS *xcs, int depth);

/**
 * The "public" tree generation function. 
 */
struct Tree** generateForest(struct xClassifierSet *pop, struct XCS *xcs)
{
  double *specs;
  int *used, i;
  struct Tree** forest;
  double *predictionArray;
  struct xClassifierSet *cll;

  assert((specs = (double *)calloc(getConditionLength(), sizeof(double)))!=0);
  assert((used = (int *)calloc(getConditionLength(), sizeof(int)))!=0);
  assert((predictionArray = (double *)calloc(getNumberOfActions(),sizeof(double)))!=0);
  assert((forest = (struct Tree **)calloc(getConditionLength()+1, sizeof(struct Tree *)))!=NULL);
  
  for(i=0; i<getConditionLength(); i++) {
    cll = copyClassifierSet(pop);
    forest[i] = getForestRecursive(cll, specs, used, predictionArray, xcs, 0);
    if(forest[i] != NULL) {
      used[forest[i]->attPos]=1;
    }else{
      freeSet(&cll);
      break;
    }
  }

  free(predictionArray);
  free(used);
  free(specs);
  return forest;
}

/**
 * Generates the tree nodes recursively.
 */
struct Tree *getForestRecursive(struct xClassifierSet *cll, double *specs, int *used, double *predArray, struct XCS *xcs, int depth)
{
  int i, length, pos;
  double max;
  struct xClassifierSet **subSets;
  struct condAttribute *conAtt;
  struct Tree *retTree;

  assert((retTree = (struct Tree *)calloc(1, sizeof(struct Tree)))!=NULL);
  retTree->representatives = cll;
  retTree->support = getNumerositySum(cll);
  retTree->depth = depth;

  /* printf("Recursion %d!", depth); */

  /* determine the predicted class of this node in the tree */
  getPredictionArray(cll, predArray, xcs);
  max=0;
  pos=-1;
  for(i=0; i<getNumberOfActions(); i++) {
    if(pos==-1 || max < predArray[i]) {
      max = predArray[i];
      pos = i;
    }
  }
  retTree->class_index = pos;
  
  /* determine the next position to split on */
  getPosSpecificity(cll, specs);
  pos=-1;
  max=0;
  length = getConditionLength();
  for(i=0; i<length; i++) {
    /* printf("%1.3f ", specs[i]);*/
    if(used[i]==0) {
      if(pos==-1 || max < specs[i]) {
	pos = i;
	max = specs[i];
      }
    }
  }
  if( pos==-1 || max==0 ) { /* nothing to split anymore */
    /*printf("NULL\n");*/
    return retTree;
  }

  for(i=0; i<getNumberOfActions(); i++) {
    if(getActionSupport(i, cll) < 1) {
      return retTree;
    }
  }

  /* pos is the one with the highest specificity */
  retTree->attPos = pos;
  used[pos] = 1;
  /* printf("Splitting on Node %d...\n",pos);*/
  
  /* partition the population */
  if(getAttributeType(pos)>1 || (getAttributeType(pos)==1 && getUpperBound(pos)-getLowerBound(pos)>1)) {
    printf("Do not support real values or integers with more than two values!!!\n");
    return retTree;
  }
  
  assert((subSets = (struct xClassifierSet **)calloc(3, sizeof(struct xClassifierSet *)))!=NULL);

  for( ; cll!=NULL && cll->cl!=NULL ; cll=cll->next) {
    conAtt = getAttribute(cll->cl->con, pos);
    if( (conAtt->type==0 && conAtt->boole == '0') || 
        (conAtt->type==1 && conAtt->upperBound == (int)getLowerBound(pos)) 
       ) {
      addNewClassifierToSet(cll->cl, &subSets[0]);
    }else if( (conAtt->type==0 && conAtt->boole == '1') ||
	      (conAtt->type==1 && conAtt->lowerBound == (int)getUpperBound(pos)) 
	      ) {
      addNewClassifierToSet(cll->cl, &subSets[1]);
    }else{
      addNewClassifierToSet(cll->cl, &subSets[2]);
    }
  }
  
  retTree->numChildren = 0;
  assert((retTree->children = (struct Tree **)calloc(3, sizeof(struct Tree*)))!=NULL);
  if(subSets[0]!=NULL) {
    /*
     * printf("(%d)%d=0:", depth, retTree->attPos);
     * printClassifierSet(zeroL);
     */
    (retTree->children)[0] = getForestRecursive(subSets[0], specs, used, predArray, xcs, depth+1);
    (retTree->numChildren)++;
  }
  if(subSets[1]!=NULL) {
    /* 
     * printf("(%d)%d=1:", depth, retTree->attPos);
     * printClassifierSet(oneL);
     */
    (retTree->children)[1] = getForestRecursive(subSets[1], specs, used, predArray, xcs, depth+1);
    (retTree->numChildren)++;
  }
  if(subSets[2]!=NULL) {
    /* 
     * printf("(%d)%d=#:", depth, retTree->attPos);
     * printClassifierSet(hashL);
     */
    (retTree->children)[2] = getForestRecursive(subSets[2], specs, used, predArray, xcs, depth+1);
    (retTree->numChildren)++;
  }
  used[retTree->attPos]=0;
  free(subSets);
  return retTree;
}




