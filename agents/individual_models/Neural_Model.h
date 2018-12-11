
//trivial model - the dna is the output

#ifndef NEURAL_MODEL_H
#define NEURAL_MODEL_H

#include"Individual_Model.h"
#include"string.h"
#include"../neural_model/Simple_Feedforward.h"

class Neural_Model : public Individual_Model
{
	public:

		//Neural_Model(int number_of_action_vars, Random* random);
		Neural_Model(int number_of_inputs, int number_of_hidden, int number_of_outputs, int number_of_hidden_layers, Random* random);
		~Neural_Model();
		//Variables derived from the Parent class
		//double *dna;
		//Random* random;
		//int number_of_observation_vars;
		//int number_of_action_vars;

		//Implementing the Interface
		void updateModel(double* dna);
		void getOutput(double* input, double* output);
		double diversity(Individual_Model* model);

		Simple_Feedforward* neuron;
};

#endif
