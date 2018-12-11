
// Real Inputs, Integer/Binary Output

#ifndef XCSF_H
#define XCSF_H

#include"Reinforcement_Agent.h"
#include"random/State_of_Art_Random.h"
#include"stdlib.h"
#include"stdio.h"
#include"time.h"
#include"genetic_operators.h"
#include"useful/useful_utils.h"
#include"string.h"

#define TOURNAMENT_SIZE 16

class XCSF : public Reinforcement_Agent
{
		void covering(double* observation);
		void coveringForAction(double* observation, int action_state);
		void coveringForEveryAction(double* observation);
		void reinforcement(double current_reward);
		void update(int* current_action_set, int action_set_size, double* observation, int match_set_size);
		void geneticAlgorithm(int* set, int set_size);
		int individualForDeletion();
		bool isMoreGeneral(int subsumer, double* tmp_dna);
		bool canSubsume(int subsumer);
		bool doesSubsume(int subsumer, double* tmp_dna);
		void copyIndividual(int dst_index, int src_index);
		void deleteOneIndividual();
		void resetIndividual(int index);

		//debug
		void printInputOutput(double* observation, double* action);

	public:

		XCSF(int population_size);
		~XCSF();

		//All Reinforcement Agents have the following commented variables, although it is not declared here!
		//double* action;
		//int number_of_observation_vars;
		//int number_of_action_vars;
		
		Random* random;
		int dna_size;
		int population_size;
		
		double** dna;
		double** pop_output;
		int* numerosity;
		int exceeding_numerosity;
		double* k;
		double* pop_fitness;
		double* pop_reward_prediction;
		double* true_reward;
		double* pop_prediction_error;
		int* pop_experience;
		int* match_set;		//store the individual's index of whom are in the match set
		int match_set_size;
		int* action_set;
		int action_set_size;
		int* previous_match_set;
		int previous_match_set_size;
		int* previous_action_set;	//usually one individual, but we can tune this to have multiple actions being given at the same time.
		int previous_action_set_size;
		double* previous_observation;
		double* previous_pop_reward_prediction;
		double* deletion_vote;

		//temporary
		double* tmp_individual;

		double** prediction_weights;
		int exploratory;

		//covering
		double* pop_match_set_size;
		int* pop_match_set_size_adjusted_times;

		//GA
		int* time_stamp;
		int step_counter;
		int trial_counter;
		double mu;	//chance of allele mutation
		double increment;	//maximum value to change by mutation (may be positive or negative)
		double CR;	//chance of mutated allele
		
		//parameters
		double beta;	//updating coefficient
		double delta;	//determines the fraction of the fitness, below which it is considered in the deletion probability
		double epsilon_zero;
		double alpha;
		double v;	//exponent used to calculate the fitness
		int max_population_size;
		unsigned int theta_GA;
		double theta_del;
		double theta_sub;
		double x_zero; 	//constant  input
		double n; 	//coefficient of update to the weights
		double gamma; 	//discount rate applied to the reward

		//Implementing the Reinforcement Agent Interface
		void init(int number_of_observation_vars, int number_of_action_vars);
		void step(double* observation, double reward);
		void print();
		double stepBestAction(double* observation);
		void endEpisode();
		void saveAgent(const char* filename);
		void loadAgent(const char* filename);

		//Integer related
		int getIntegerOutput(double* dna);
		int number_of_states;

		//debug
		FILE* fp;
		void printToFile(int match_set_size);
		int new_individual;
};

#endif
