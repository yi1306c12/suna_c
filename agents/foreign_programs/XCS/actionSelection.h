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
/     Header for the stuff that is related to the action selection procedure.
*/

void getPredictionArray(struct xClassifierSet *ms, double *pa, struct XCS *xcs);
int learningActionWinner(double *predictionArray, double exploreProb);
int randomActionWinner(double *predictionArray);
int deterministicActionWinner(double *predictionArray);
int rouletteWheelActionWinner(double *predictionArray);
