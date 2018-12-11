
// Use SOM to do some niching with the input, neural networks to process information and
// evolution to improve them

#ifndef NOTC_H
#define NOTC_H

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

struct complex_cell_contents
{
	int best_index[NUMBER_OF_POPULATIONS][BEST_SOLUTIONS_SIZE];
	double best_fitness[NUMBER_OF_POPULATIONS][BEST_SOLUTIONS_SIZE];
	int best_counter[NUMBER_OF_POPULATIONS][BEST_SOLUTIONS_SIZE];

	int novel_index[NUMBER_OF_POPULATIONS][NOVEL_SOLUTIONS_SIZE];
	double novel_fitness[NUMBER_OF_POPULATIONS][NOVEL_SOLUTIONS_SIZE];
	int novel_counter[NUMBER_OF_POPULATIONS][NOVEL_SOLUTIONS_SIZE];

	int index_of_last_acted;	//population index, of the individual who acted last in this cell
	int when_last_acted;		//generation or related number, when the last acted from this cell
	
	double experience;
	double accum_experience;
};

using namespace std;

//#define HALL_OF_FAME_SIZE BEST_SOLUTIONS_SIZE/2
//#define HALL_OF_FAME_SIZE (BEST_SOLUTIONS_SIZE - 1)
//#define HALL_OF_FAME_SIZE BEST_SOLUTIONS_SIZE
//#define HALL_OF_FAME_SIZE 1
#define TEAM_POPULATION 0
#define INDIVIDUALISTIC_POPULATION 1

class NOTC : public Reinforcement_Agent
{
	public:

		//Novelty_Organizing_Neurons(int best_solutions_size, int novel_solutions_size, int som_width, int som_height);
		NOTC(Random* random);
		~NOTC();

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
		int gen;
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
		void printCellContents(complex_cell_contents* c);
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
		double maxFitness(complex_cell_contents* contents);
		void decrement(int individual);
		void printIndividualFitness(int nmap_index, complex_cell_contents* c);
		void printMap(const char* filename);

		//GA
		void localGeneticAlgorithm(int nmap_index, complex_cell_contents* c);
		bool runOrNotRunIndividualisticGA(complex_cell_contents* c);
		void sortByFitness();
		void sortByFitness(complex_cell_contents* c, int population_size);
		void removeNovel();
		void removeNovel(complex_cell_contents* c, int population_index);
		void selectionPressure(complex_cell_contents* c, int population_index);
		void createNovelIndividuals(complex_cell_contents* c, int population_index);
		int createRandomIndividual();
		void clearExperience();
		int createOffspring(complex_cell_contents* c, int population_index);
		int createOffspring(complex_cell_contents* c, int novel_index, int population_index);
		//void printActionDistribution();
		int randomIndividual();
		void checkDNALimits(double* dna, int dna_size);
		double CR;

		//Implementing the Reinforcement Agent Interface
		void init(int number_of_observation_vars, int number_of_action_vars);
		void step(double* observation, double reward);
		void print();
		double stepBestAction(double* observation);
		void saveAgent(const char* filename);
		void loadAgent(const char* filename);
		void endEpisode(double reward);

		void swapTeams(int a, int b);
		bool checkHierarchicalAscension(int hof_index);

		//team variables and methods
		int episode;
		double current_team_fitness;
		int team_actions;
		int hall_of_fame[BEST_SOLUTIONS_SIZE][NOVELTY_POP];	//NOVELTY_POP is the number of individuals in a team
		double hall_of_fame_fitness[BEST_SOLUTIONS_SIZE];
		double hall_of_fame_count[BEST_SOLUTIONS_SIZE];
		
		void checkHallOfFame(double fitness);
		void teamEvolution();
		void copyLastTeamToHallOfFame(int worst_team_index);
		
};

#endif
		
