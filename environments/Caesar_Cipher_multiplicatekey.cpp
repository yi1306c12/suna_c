#include"Caesar_Cipher_multiplicatekey.hpp"

#include"../parameters.h"
#include"random/State_of_Art_Random.h"
#include<vector>//std::vector
#include<functional>//std::multiplies
#include<cmath>//fabs
#include<numeric>//std::accumulate

Caesar_Cipher_multiplicatekey::Caesar_Cipher_multiplicatekey(Random* random, int const number_of_key):
    Caesar_Cipher(random), 
    number_of_key(number_of_key),
    max_value_of_each_key(
        pow(max_value_of_key, 1./static_cast<double>(number_of_key))
        ),
    keys(std::vector<double>(number_of_key))
{
    this->random = random;
}

void Caesar_Cipher_multiplicatekey::start(int &number_of_observation_vars, int& number_of_action_vars)
{
	number_of_observation_vars= 1+number_of_key;
	this->number_of_observation_vars= 1+number_of_key;
	observation= (double*) malloc(number_of_observation_vars*sizeof(double));

	number_of_action_vars=1;
	this->number_of_action_vars=1;

	restart();
}

double Caesar_Cipher_multiplicatekey::restart()
{
    trial++;

    //set new message
    current_message= random->uniform(0., max_value_of_message);
    observation[0]= current_message;
    message_counter= 0;

    for(int i=0; i < number_of_key; ++i)
    {
        keys[i] = random->uniform(1., max_value_of_each_key);
        observation[i+1] = keys[i];
    }
    key = std::accumulate(keys.begin(), keys.end(), 1., std::multiplies<double>());

    return 0.;
}

double Caesar_Cipher_multiplicatekey::step(double* action)
{
	// initial reward
	if(action==NULL)
	{
		return 0;
	}

    double reward = -fabs(
        static_cast<int>(action[0]) - static_cast<int>(current_message + key)
    );

    if(message_counter > max_steps_of_messages)
    {
        restart();
    }
    else
    {
        current_message= random->uniform(0., max_value_of_message);
        observation[0]= current_message;
        ++message_counter;
    }
    return reward;
}