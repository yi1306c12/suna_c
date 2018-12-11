
//Mysterious = Random ;P

#ifndef MYSTERIOUS_AGENT_H
#define MYSTERIOUS_AGENT_H

#include"Reinforcement_Agent.h"
#include"random/State_of_Art_Random.h"
#include"stdlib.h"
#include"stdio.h"
#include"time.h"
#include"foreign_programs/XCS/xcs.h"

class XCS_Butz : public Reinforcement_Agent
{
	public:

		XCS_Butz();
		~XCS_Butz();

		//All Reinforcement Agents have the following commented variables, although it is not declared here!
		//double* action;
		//int number_of_observation_vars;
		//int number_of_action_vars;
		Random* random;
  		struct XCS *xcsDefault;

		//Implementing the Reinforcement Agent Interface
		void init(int number_of_observation_vars, int number_of_action_vars);
		void step(double* observation, double reward);
		void print();
		double stepBestAction(double* observation);
		void endEpisode();
		void saveAgent(const char* filename);
		void loadAgent(const char* filename);

};

#endif
