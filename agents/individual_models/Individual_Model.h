
#ifndef INDIVIDUAL_MODEL_H
#define INDIVIDUAL_MODEL_H

#include"random/State_of_Art_Random.h"

class Individual_Model
{
	public:

		double *dna;
		int dna_size;
		Random* random;
		//int number_of_observation_vars;
		//int number_of_action_vars;
		int numerosity;		//number of indexes indexing this individual

		virtual void updateModel(double* dna)=0;
		virtual void getOutput(double* input, double* output)=0;
		virtual double diversity(Individual_Model* model)=0;
		void increment()
		{
			numerosity++;
		}
		void decrement()
		{
			numerosity--;
		}
};

#endif
