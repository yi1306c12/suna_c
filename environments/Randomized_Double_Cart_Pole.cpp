#include"Randomized_Double_Cart_Pole.hpp"

Randomized_Double_Cart_Pole::Randomized_Double_Cart_Pole(Random* random, double const mu, double const sigma):
    Double_Cart_Pole(random), mu(mu), sigma(sigma)
{}

double Randomized_Double_Cart_Pole::step(double* action)
{
    double const reward = Double_Cart_Pole::step(action);
    for(int i = 0; i < number_of_observation_vars; ++i)
    {
        observation[i] = random->gaussian(observation[i]+mu, sigma);
    }
    return reward;
}