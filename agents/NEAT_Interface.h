
//Mysterious = Random ;P

#ifndef NEAT_Interface_H
#define NEAT_Interface_H

#include"Reinforcement_Agent.h"
#include"random/State_of_Art_Random.h"
#include"stdlib.h"
#include"stdio.h"
#include"time.h"

#include"../parameters.h"
#include <iostream>
#include <vector>
#include "neat/neat.h"
#include "neat/population.h"
#include "neat/experiments.h"

class NEAT_Interface : public Reinforcement_Agent
{
	public:

		NEAT_Interface(Random* random, const char* filename_with_parameters);
		~NEAT_Interface();
	

		//originally from NEAT's
		vector<Organism*>::iterator curorg;
		Population *pop;
		//Organism *champ;
		Network *champ_net;
		Genome *start_genome;
		char curword[20];
		int id;

		double* modified_input;
		int step_counter;

		ostringstream *fnamebuf;
		int gen;

		int expcount;
		int status;
		int totalevals;
		int samples;  //For averaging
		int runs[1];

		//All Reinforcement Agents have the following commented variables, although it is not declared here!
		//double* action;
		//int number_of_observation_vars;
		//int number_of_action_vars;
		
		Random* random;

		void findBestIndividual();
		void createInitialGene(const char* filename, int number_of_inputs, int number_of_outputs);

		//Implementing the Reinforcement Agent Interface
		void init(int number_of_observation_vars, int number_of_action_vars);
		void step(double* observation, double reward);
		void print();
		double stepBestAction(double* observation);
		void endEpisode(double reward);
		void saveAgent(const char* filename);
		void loadAgent(const char* filename);

};

#endif
