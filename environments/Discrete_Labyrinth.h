
// A closed empty maze

#ifndef GENERAL_LABYRINTH
#define GENERAL_LABYRIMTH

#include"Reinforcement_Environment.h"
#include"../agents/Reinforcement_Agent.h"
#include"random/State_of_Art_Random.h"
#include"stdlib.h"
#include"stdio.h"
#include"time.h"
#include"useful/read_csv.h"
#include"useful/useful_utils.h"
#include"../parameters.h"

class Discrete_Labyrinth : public Reinforcement_Environment
{
	public:

		Discrete_Labyrinth(int trials_to_change_maze_states, Random* random);
		~Discrete_Labyrinth();
		
		//All Reinforcement Problems have the observartion variable, although it is not declared here!
		//double* observation;	
		//int number_of_observation_vars;
		//int number_of_action_vars;
		
		Random* random;

		double*** map;

		double x_pos;
		double y_pos;

		int* size_x;
		int* size_y;

		bool noisy_input;	//observer input is noisy, varies uniformly 5%
		
		//related to Maze States (maze will change over trials)
		int current_maze_state;
		int number_of_maze_states;
		int trials_to_change_maze_states;

		double max_velocity_per_step;

		int step_counter;
		double default_reward;	//usual reward when the system is idle or for common inputs

		void printAgentBehavior(Reinforcement_Agent* agent);
		void convertMap(int state);

		//debug
		FILE* fp;
		bool print_trajectory;
		void printTrajectory(FILE* fp, double* observation, int size);

		//Implementing the Reinforcement Problem Interface
		void start(int &number_of_observation_vars, int& number_of_action_vars);
		double step(double* action);
		double restart();
		void print();
};
#endif
