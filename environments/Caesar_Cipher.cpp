#include"Caesar_Cipher.h"

Caesar_Cipher::Caesar_Cipher(Random* random):
max_steps_of_messages(100)
{
	this->random= random;
	trial=-1;	//the first trial will be zero, because restart() is called making it increment
	MAX_STEPS= 9999;
}

Caesar_Cipher::~Caesar_Cipher()
{

}

void Caesar_Cipher::start(int &number_of_observation_vars, int& number_of_action_vars)
{
	number_of_observation_vars= 2;
	this->number_of_observation_vars= 2;
	observation= (double*) malloc(number_of_observation_vars*sizeof(double));

	number_of_action_vars=1;
	this->number_of_action_vars=1;

	restart();
}

double Caesar_Cipher::step(double* action)
{
	// initial reward
	if(action==NULL)
	{
		return 0;
	}


	double reward= - fabs(((int)action[0]) - (int)(current_message + key));
	//double reward= - fabs(((int)action[0])%25 - (int)(current_message + key)%25);



	if(message_counter > max_steps_of_messages) 
	{
		restart();
		//return 1/(x*x + theta*theta + 0.001);             
		return reward;
	}
	else
	{
		//set new message
		current_message= random->uniform(0.0, max_value_of_message);

		observation[0]= current_message;
		message_counter++;
	}

	return reward;
}

double Caesar_Cipher::restart()
{
	trial++;
	
	//set new message
	current_message= random->uniform(0.0, max_value_of_message);
	observation[0]= current_message;
	message_counter=0;
	
	key= random->uniform(0.0, max_value_of_key);
	observation[1]= key;

	return 0;
}

void Caesar_Cipher::print()
{

}



