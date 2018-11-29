
// A closed empty maze

#ifndef CONTINUOUS_EMPTY_MAZE
#define CONTINUOUS_EMPTY_MAZE

#include"Reinforcement_Environment.h"
#include"random/State_of_Art_Random.h"
#include"stdlib.h"
#include"stdio.h"
#include"time.h"

class Continuous_Empty_Maze : public Reinforcement_Environment
{
	public:

		Continuous_Empty_Maze();
		~Continuous_Empty_Maze();
		
		//All Reinforcement Problems have the observartion variable, although it is not declared here!
		//double* observation;	
		//int number_of_observation_vars;
		//int number_of_action_vars;
		
		Random* random;

		double x;
		double y;

		double size_x;
		double size_y;

		double max_velocity_per_step;

		double target_x;
		double target_y;

		int step_counter;

		//Implementing the Reinforcement Problem Interface
		void start(int &number_of_observation_vars, int& number_of_action_vars);
		double step(double* action);
		double restart();
};
#endif
