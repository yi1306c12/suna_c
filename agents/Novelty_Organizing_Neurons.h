
// Use SOM to do some niching with the input, neural networks to process information and
// evolution to improve them

#ifndef NOVELTY_ORGANIZING_NEURONS_H
#define NOVELTY_ORGANIZING_NEURONS_H

#include"Reinforcement_Agent.h"
#include"random/State_of_Art_Random.h"
#include"stdlib.h"
#include"stdio.h"
#include"time.h"
#include"individual_models/Trivial_Model.h"
#include"individual_models/Neural_Model.h"
#include"self_organized_systems/Novelty_Map.h"
#include"useful/useful_utils.h"
#include<stack>
#include"genetic_operators.h"
#include"../parameters.h"
//#include"incremental_classifiers/Incremental_Cascade_Correlation.h"


using namespace std;

#define HALL_OF_FAME_SIZE BEST_SOLUTIONS_SIZE/2
//#define HALL_OF_FAME_SIZE (BEST_SOLUTIONS_SIZE - 1)
//#define HALL_OF_FAME_SIZE BEST_SOLUTIONS_SIZE
//#define HALL_OF_FAME_SIZE 1

class Novelty_Organizing_Neurons : public Reinforcement_Agent
{
	public:

		//Novelty_Organizing_Neurons(int best_solutions_size, int novel_solutions_size, int som_width, int som_height);
		Novelty_Organizing_Neurons(Random* random);
		~Novelty_Organizing_Neurons();

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
		bool exploratory;
		bool use_surrogate;
#ifdef PRINT_SURROGATE_ERROR	
		FILE* surrogate_error_fp;
#endif			

		double widrow_coefficient;
		double gamma;	//used in the reinforcement learning equation R= reward + gamma*Q_max

		Individual_Model** population;
		Novelty_Map* nmap;
		//Incremental_Classifier* incremental_classifier;

		//temporary 
		double* tmp_action;

		//previous Action
		double* previous_observation_action;	//previous observation and action arrays concatenated
		int prev_nmap_index;		//position activated in the som for the previous action
		bool prev_exploratory;	//true - previous action came from the novel vector (false - best vector)
		int prev_model;		//the index of the model in the respective vector

		//debug
		void printExperience();
		void printCellContents(cell_contents* c);
		void printPopulationSize();
		void initPrint();
		void finishPrint();
		void printDNA();
		FILE* population_fp;
#ifdef	PRINT_DIVERSITY		
		FILE* diversity_fp;
		void printAverageDiversity();
#endif		
#ifdef	PRINT_FITNESS_TABLE	
		FILE* fitness_fp;
#endif	
		FILE* evo_fp;
		int trial;

		//auxiliary
		void surrogateModelUpdate(double reward);
		int chebyshevDistance(int dx, int dy);
		void createCellContents(int nmap_index);
		void storeAction(int nmap_index,int model_index, double* observation, double* action);
		void reinforcement(double reward);
		double maxFitness(cell_contents* contents);
		void decrement(int individual);
		void printIndividualFitness(int nmap_index, cell_contents* c);

		//GA
		void localGeneticAlgorithm(int nmap_index, cell_contents* c);
		bool runOrNotRunGA(cell_contents* c);
		void sortByFitness();
		void sortByFitness(cell_contents* c);
		void removeNovel();
		void removeNovel(cell_contents* c);
		void selectionPressure(cell_contents* c);
		void createNovelIndividuals(cell_contents* c);
		int createRandomIndividual();
		void clearExperience();
		int createOffspring(cell_contents* c);
		int createOffspring(cell_contents* c, int novel_index);
		void printActionDistribution();
		int randomIndividual();
		void checkDNALimits(double* dna, int dna_size);
		double CR;

		//Implementing the Reinforcement Agent Interface
		void init(int number_of_observation_vars, int number_of_action_vars);
		void step(double* observation, double reward);
		void print();
		void printMap(const char* filename);
		double stepBestAction(double* observation);
		void saveAgent(const char* filename);
		void loadAgent(const char* filename);

		void endEpisode(double reward);

		//team variables and methods
		int episode;
		double current_team_fitness;
		int team_actions;
		int hall_of_fame[HALL_OF_FAME_SIZE][NOVELTY_POP];	//NOVELTY_POP is the number of individuals in a team
		double hall_of_fame_fitness[HALL_OF_FAME_SIZE];
		
		void checkHallOfFame(double fitness);
		void Evolution();
		void copyLastTeamToHallOfFame(int worst_team_index);
		
};

#endif
		
