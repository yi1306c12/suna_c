
// Use SOM to do some niching with the input, neural networks to process information and
// evolution to improve them

#ifndef SELF_ORGANIZING_NEURONS_H
#define SELF_ORGANIZING_NEURONS_H

#include"Reinforcement_Agent.h"
#include"random/State_of_Art_Random.h"
#include"stdlib.h"
#include"stdio.h"
#include"time.h"
#include"individual_models/Trivial_Model.h"
#include"individual_models/Neural_Model.h"
#include"self_organized_systems/Som_Map.h"
#include"useful/useful_utils.h"
#include<stack>
#include"genetic_operators.h"
#include"../parameters.h"
//#include"incremental_classifiers/Incremental_Cascade_Correlation.h"


using namespace std;



class Self_Organizing_Neurons : public Reinforcement_Agent
{
	public:

		//Self_Organizing_Neurons(int best_solutions_size, int novel_solutions_size, int som_width, int som_height);
		Self_Organizing_Neurons(int som_width, int som_height, Random* random);
		~Self_Organizing_Neurons();

		//All Reinforcement Agents have the following commented variables, although it is not declared here!
		//double* action;
		//int number_of_observation_vars;
		//int number_of_action_vars;
		
		Random* random;

		stack<int> deleted_individuals;	//stack of population indices where individuals were deleted

		// Every Cell will have two vectors inside:
		// -a vector of best solutions (for giving the best answer)
		// -a vector of novel solutions (for exploration)
		// the population is basically the maximum number of solutions a SOM map can address
		// with each cell having these two vectors. Basically: SOM's size * (best solutions + novel solutions).
		int max_population_size;
		int population_size;
		int best_solutions_size;
		int novel_solutions_size;
		int som_width;
		int som_height;
		bool exploratory;
		bool use_surrogate;
		int surrogate_counter;
#ifdef PRINT_SURROGATE_ERROR	
		FILE* surrogate_error_fp;
#endif			

		double widrow_coefficient;
		double gamma;	//used in the reinforcement learning equation R= reward + gamma*Q_max

		Individual_Model** population;
		Som_Map* som;
		//Incremental_Classifier* incremental_classifier;

		//temporary 
		double* tmp_action;

		//previous Action
		double* previous_observation_action;	//previous observation and action arrays concatenated
		int prev_som_x;		//position activated in the som for the previous action
		int prev_som_y;
		bool prev_exploratory;	//true - previous action came from the novel vector (false - best vector)
		int prev_model;		//the index of the model in the respective vector

		//debug
		void printExperience();
		void printCellContents(cell_contents* c);
		void printPopulationSize();
		void initPrint();
		void finishPrint();
		void printDNA();
		void printSOM(const char* filename);
		FILE* population_fp;
#ifdef	PRINT_DIVERSITY		
		FILE* diversity_fp;
		void printAverageDiversity();
#endif		

		//auxiliary
		void surrogateModelUpdate(double reward);
		int chebyshevDistance(int dx, int dy);
		void createCellContents(int i_index, int j_index);
		void getMostExperientNeighbor(int i_index, int j_index, int& neighbor_x, int& neighbor_y);
		void storeAction(int i_index, int j_index, int model_index, double* observation, double* action);
		void reinforcement(double reward);
		double maxFitness(cell_contents* contents);
		void decrement(int individual);
		void printIndividualFitness(int i_index, int j_index, cell_contents* c);

		//GA
		void localGeneticAlgorithm(int i_index, int j_index, cell_contents* c);
		bool runOrNotRunGA(cell_contents* c);
		void sortByFitness();
		void sortByFitness(cell_contents* c);
		void removeNovel();
		void removeNovel(cell_contents* c);
		void selectionPressure(cell_contents* c);
		void createNovelIndividuals(int i_index, int j_index, cell_contents* c);
		int createRandomIndividual();
		void clearExperience();
		int createOffspring(int i_index, int j_index, cell_contents* c);
		int createOffspring(int i_index, int j_index, cell_contents* c, int novel_index);
		void printActionDistribution();
		int randomIndividual();
		int getRandomNeighborOrRandomIndividual(int i_index, int j_index);
		void checkDNALimits(double* dna, int dna_size);
		double CR;

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
		
