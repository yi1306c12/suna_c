#ifndef CACLA_H
#define CACLA_H

# include <string>
# include <math.h>
# include <vector>
# include <time.h>
# include "Algorithm.h"
# include "cNeuralNetwork.h"
# include "../Reinforcement_Agent.h"
#include"random/State_of_Art_Random.h"
//#include"../"
	
#define NUMBER_OF_HIDDEN_NODES_Q 10
#define NUMBER_OF_HIDDEN_NODES_V 10
#define CACLA_GAMMA 0.99
#define LEARNING_RATE_A 0.00001
#define LEARNING_RATE_V 0.001
#define GAUSSIAN_SIGMA 500.0
#define EGREEDY_EPSILON 1.0

enum{EGREEDY_EXPLORATION, GAUSSIAN_EXPLORATION};
#define CACLA_EXPLORATION_TYPE GAUSSIAN_EXPLORATION
//#define CACLA_EXPLORATION_TYPE EGREEDY_EXPLORATION

class Cacla : public Algorithm, public Reinforcement_Agent
{
    public:
        Cacla(Random* random) ;
        ~Cacla() ;
        void getMaxAction( double * state, double * action ) ;
        void getRandomAction( double * state, double * action ) ;
        void explore( double * state, double * action) ;
        void update( double * st, double * action, double reward, double * st_, bool endOfEpisode) ;
		
	//double* action;
	//int number_of_observation_vars;
	//int number_of_action_vars;
	double* previous_observation;
	Random* random;

	void init(int number_of_observation_vars, int number_of_action_vars);
	void step(double* observation, double reward);
	double stepBestAction(double* observation);
	void print();
	void endEpisode();


	//private:
	

        void gaussian( double * state, double * action, double sigma ) ;

        double gaussianRandom() ;
        double g1, g2 ;
        bool storedGauss ;

        int nHiddenQ, nHiddenV ;
        double epsilon, sigma ;
        cNeuralNetwork * ANN ;
        cNeuralNetwork * VNN ;
        double * VTarget ;
	double gamma;
	double learningRate[2];
	
	//handicap
	bool endOfEpisode;
};

#endif //CACLA_H
