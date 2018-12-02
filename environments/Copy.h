/********************************************************************************
 *
 * 	The Copy environment tests if an algorithm can learn to copy a sequence 
 * 	of vectors after a delimiter is passed
 *
 * *****************************************************************************/

#ifndef COPY
#define COPY

#include"../parameters.h"
#include"Reinforcement_Environment.h"
#include"../agents/Reinforcement_Agent.h"
#include"random/State_of_Art_Random.h"
#include"stdlib.h"
#include"stdio.h"
#include"math.h"


class Copy : public Reinforcement_Environment
{

	public:
		Copy(int input_symbol_possibilities, int number_of_inputs, int sequence_length, Random* random);
		~Copy();
		
		//Inherited variables
		//double* observation;		
		//int number_of_observation_vars;
		//int number_of_action_vars;
		//int trial;
		//int MAX_STEPS;
		
		Random* random;
	
		int input_symbol_possibilities;
		int number_of_inputs;
		int sequence_length;
		int sequence_counter;
		bool verbose;

		int** input_data;
		bool normalized_observation;
		bool normalized_action;

		//API
		void start(int &number_of_observation_vars, int& number_of_action_vars);
		double step(double* action);
		double restart();
		void print();
		bool set(int feature);
};

#endif
