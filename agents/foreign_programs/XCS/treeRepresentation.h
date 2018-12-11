#include "xcsMacros.h"
#include "classifierList.h"
#include "actionSelection.h"
//#include "env.h"

struct Tree{
  int attPos;
  int support;
  int depth;
  struct Tree** children;
  int numChildren;
  int class_index;
  struct xClassifierSet *representatives;
};

struct Tree* generateTree(struct xClassifierSet *cll, struct XCS *xcs);
void deleteTree(struct Tree **tree);

double testTree(struct Tree *tree);
int getClassification(struct Tree *tree, struct attributes *probInstance);

void fprintAllTree(FILE *fp, struct Tree *tree, int type, struct XCS *xcs);
void fprintDOTTree(FILE *fp, struct Tree *tree, int type);


struct Tree** generateForest(struct xClassifierSet *cll, struct XCS *xcs);
void deleteForest(struct Tree ***forest);

void fprintAllForest(FILE *fp, struct Tree **forest, int type, struct XCS *xcs);
void fprintDOTForest(FILE *fp, struct Tree **forest, int type);

