
#include"Frog_Fly.h"

Frog_Fly::Frog_Fly(bool usual_version)
{
	this->usual_version= usual_version;
	trial=0;
}

Frog_Fly::~Frog_Fly()
{
	free(observation);
}

	
//set the number of observable vars and number of actions
void Frog_Fly::start(int &number_of_observation_vars, int& number_of_action_vars)
{
	number_of_observation_vars= 1;
	number_of_action_vars=1;

	this->number_of_observation_vars= number_of_observation_vars;
	this->number_of_action_vars= number_of_action_vars;

	random= new State_of_Art_Random(time(NULL));


	//allocate observation
	observation= (double*) malloc(number_of_observation_vars*sizeof(double));

}
	
//return a reward
//and update environment's variable
double Frog_Fly::step(double* action)
{
	////////////////// Initial Reward ///////////////////
	
	//set the value of the initial reward
	if(action==NULL)
	{
		return 0;
	}

	////////////////// Return Reward ///////////////////

	double reward=0.0;

	//check for out of bound action (the jumping force is limited)
	if(action[0] > 1.0)
	{
		action[0]=1.0;
	}
	if(action[0] < 0.0)
	{
		action[0]=0.0;
	}


	//usual frog problem
	if(usual_version)
	{
		if(observation[0] + action[0] < 1.0)
		{
			reward= (observation[0] + action[0]);
		}
		// action + action >= 1
		else
		{
			reward= (2.0 - observation[0] - action[0]);
		}
	}
	else
	//modified frog problem
	{
		if(action[0] < -log(observation[0]))
		{
			reward= observation[0]*exp(action[0]);
		}
		// action + action >= 1
		else
		{
			reward= (1.0/observation[0])*exp(-action[0]);
		}
	}

	/////////////// Update observation///////////////////////
	// The new observation is unrelated with the previous action 
	
	distance_from_fly= random->uniform(0.0, 1.0);

	if(usual_version)
	{
		observation[0]= 1.0 - distance_from_fly;
	}
	else
	{
		observation[0]= exp(-distance_from_fly);
	}
	
	/////////////////////////////////////////////////////
	
	trial++;

	return reward;
	
}

double Frog_Fly::restart()
{
	return 0.0;
}
