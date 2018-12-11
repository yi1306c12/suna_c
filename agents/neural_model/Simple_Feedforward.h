#ifndef SIMPLE_FEEDFORWARD
#define SIMPLE_FEEDFORWARD

#include"graph/Graph_Node.h"
#include"random/State_of_Art_Random.h"
#include"time.h"
#include"math.h"
#include"string.h"
#include"useful/useful_utils.h"

class Simple_Feedforward
{
		void allocateWithRandomWeights();
		void allocateWithoutSettingWeights();
	
	public:

		Simple_Feedforward(int number_of_inputs, int number_of_hidden, int number_of_outputs, int number_of_hidden_layers, Random* random);
		~Simple_Feedforward();
		
		int number_of_inputs;
		int number_of_hidden;
		int number_of_hidden_layers;
		int number_of_outputs;

		Graph_Node** input;
		Graph_Node*** hidden;
		Graph_Node** output;
		double** bias;
		double* bias_output;
		double** alpha;

		Random* random;

		void activate(double* stimulus, double* response);
		void setWeights(double* dna);
		void print();
		int numberOfWeightsForTheArchitecture();
	
};

#endif

