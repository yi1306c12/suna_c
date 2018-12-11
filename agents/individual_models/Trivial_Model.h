
//trivial model - the dna is the output

#ifndef TRIVIAL_MODEL_H
#define TRIVIAL_MODEL_H

#include"Individual_Model.h"
#include"string.h"

class Trivial_Model : public Individual_Model
{
	public:

		Trivial_Model(int number_of_action_vars, Random* random);
		~Trivial_Model();
		//Variables derived from the Parent class
		//double *dna;
		//Random* random;
		//int number_of_observation_vars;
		//int number_of_action_vars;

		//Implementing the Interface
		void updateModel(double* dna);
		void getOutput(double* input, double* output);
		double diversity(Individual_Model* model);
};

#endif
