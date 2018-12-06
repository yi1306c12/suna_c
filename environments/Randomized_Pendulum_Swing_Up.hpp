#ifndef RANDOMIZED_PENDULUM_SWING_UP
#define RANDOMIZED_PENDULUM_SWING_UP

#include"Pendulum_Swing_Up.h"

class Randomized_Pendulum_Swing_Up : public Pendulum_Swing_Up
{
    double const mu;
    double const sigma;
public:
    Randomized_Pendulum_Swing_Up(Random* random, double const mu, double const sigma);
    double step(double* action);
};

#endif