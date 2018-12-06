#include"Randomized_Pendulum_Swing_Up.hpp"


Randomized_Pendulum_Swing_Up::Randomized_Pendulum_Swing_Up(Random* random, double const mu, double const sigma):
    Pendulum_Swing_Up(random), mu(mu), sigma(sigma)
{}

double Randomized_Pendulum_Swing_Up::step(double* action)
{
    double const reward = Pendulum_Swing_Up::step(action);
    for(int i = 0; i < number_of_observation_vars; ++i)
    {
        observation[i] = random->gaussian(observation[i]+mu, sigma);
    }
    return reward;
}