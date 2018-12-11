
#ifndef INCREMENTAL_CASCADE_CORRELATION_H
#define INCREMENTAL_CASCADE_CORRELATION_H

#include "fann.h"
#include"Incremental_Classifier.h" 
#include"random/State_of_Art_Random.h"

class Incremental_Cascade_Correlation : public Incremental_Classifier
{
	public:
		Incremental_Cascade_Correlation(int number_of_inputs, int number_of_outputs, Random* random);
		~Incremental_Cascade_Correlation();
	
		int number_of_inputs;
		int number_of_outputs;
		struct fann *ann;
		struct fann_train_data *data;
		unsigned int data_counter;
		unsigned int window_size;
		fann_type* fann_output;
		fann_type* fann_input;
		Random* random;

		void incrementSamples(double* input, double* output);	
		double* predict(double* input);
		struct fann_train_data *alloc(unsigned int samples, unsigned int num_input, unsigned int num_output);
};

#endif
