
#ifndef CAESAR_CIPHER_H
#define CAESAR_CIPHER_H

#include"../parameters.h"
#include"Reinforcement_Environment.h"
#include"../agents/Reinforcement_Agent.h"
#include"random/State_of_Art_Random.h"
#include"stdlib.h"
#include"stdio.h"
  	
#define FORCE_MAG 10.0
   
class Caesar_Cipher : public Reinforcement_Environment
{
	public:

		Caesar_Cipher(Random* random);
		~Caesar_Cipher();
		
		//All Reinforcement Problems have the observartion variable, although it is not declared here!
		//double* observation;		
		//int number_of_observation_vars;
		//int number_of_action_vars;
		//int trial;
		
		//Reinforcement Problem API
		void start(int &number_of_observation_vars, int& number_of_action_vars);
		double step(double* action);
		double restart();
		void print();
	
		Random* random;

		int const max_steps_of_messages;
		int message_counter;
		double key;
		double current_message;

		double const max_value_of_message = 25.;
		double const max_value_of_key = 25.;
};

#endif
