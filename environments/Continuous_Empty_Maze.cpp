
#include"Continuous_Empty_Maze.h"

Continuous_Empty_Maze::Continuous_Empty_Maze()
{

	target_x= 1.0;
	target_y= 1.0;

	size_x= 1.0;
	size_y= 1.0;

	trial=0;

	max_velocity_per_step= 0.05;

}

Continuous_Empty_Maze::~Continuous_Empty_Maze()
{
	free(observation);
}

	
//set the number of observable vars and number of actions
void Continuous_Empty_Maze::start(int &number_of_observation_vars, int& number_of_action_vars)
{
	number_of_observation_vars= 2;
	number_of_action_vars=2;

	this->number_of_observation_vars= number_of_observation_vars;
	this->number_of_action_vars= number_of_action_vars;

	random= new State_of_Art_Random(time(NULL));


	//allocate observation
	observation= (double*) malloc(number_of_observation_vars*sizeof(double));
	
	//random initial state
	x=random->uniform(0.0,1.0);
	y=random->uniform(0.0,1.0);

	observation[0]=x;
	observation[1]=y;

	step_counter=0;
}
	
//return a reward
//and update environment's variable
double Continuous_Empty_Maze::step(double* action)
{
	int i;

	////////////////// Initial Reward ///////////////////
	
	//set the value of the initial reward
	if(action==NULL)
	{
		return -0.5;
	}

	////////////////// Return Reward ///////////////////

	double reward=0.0;

	//check for out of bound action (the velocity is limited)
	for(i=0;i < number_of_action_vars;++i)
	{
		if(action[i] > max_velocity_per_step)
		{
			action[i]= max_velocity_per_step;
		}
		if(action[i] < -max_velocity_per_step)
		{
			action[i]= -max_velocity_per_step;
		}
	}

	//update position
	x+= action[0];
	y+= action[1];

	//check for out of bound states (the maze is closed)
	if(x < 0.0)
	{
		x=0.0;
	}
	if(x > size_x)
	{
		x= size_x;
	}
	if(y < 0.0)
	{
		y=0.0;
	}
	if(y > size_y)
	{
		y= size_y;
	}

	//Compute reward
	if(x >= target_x && y >= target_y)
	{
		reward= 0.0;//1000.0;// - (double)step_counter*0.001;
	
		//random initial state
		x=random->uniform(0.0,1.0);
		y=random->uniform(0.0,1.0);

		step_counter=0;
		trial++;
	}
	else
	{
		reward= -0.5;
	}

	/////////////// Update observation///////////////////////
	
	//set the current observation
	observation[0]= x;
	observation[1]= y;

	/////////////////////////////////////////////////////

	step_counter++;

	return reward;
	
}

//restart maze problem: set the individual position at a random initial position
double Continuous_Empty_Maze::restart()
{
	//random initial state
	x=random->uniform(0.0,1.0);
	y=random->uniform(0.0,1.0);

	//set the current observation
	observation[0]= x;
	observation[1]= y;
		
	step_counter=0;
	trial++;

	//return -1000.0;
	return -0.5;
}
