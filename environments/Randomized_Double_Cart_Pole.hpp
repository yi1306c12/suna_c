#ifndef RANDOMIZED_DOUBLE_CART_POLE
#define RANDOMIZED_DOUBLE_CART_POLE

#include"Double_Cart_Pole.h"

class Randomized_Double_Cart_Pole:public Double_Cart_Pole
{
    double const mu;
    double const sigma;
public:
    Randomized_Double_Cart_Pole(Random* random, double const mu, double const sigma);
    double step(double* action);
};

#endif