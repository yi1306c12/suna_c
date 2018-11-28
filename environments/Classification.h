
#ifndef CLASSIFICATION_H
#define CLASSIFICATION_H

#include"stdlib.h"
#include"stdio.h"
#include"string.h"
#include"useful/read_csv.h"
#include"useful/useful_utils.h"
#include"../agents/Reinforcement_Agent.h"
#include"Reinforcement_Environment.h"
#include"../parameters.h"

class Classification : public Reinforcement_Environment
{
	public:

		Classification(const char* filename);
		~Classification();

		//All Reinforcement Problems have the observartion variable, although it is not declared here!
		//double* observation;		
		//int number_of_observation_vars;
		//int number_of_action_vars;
		//int trial;
		
		double** dataset;
		int number_of_samples;
		int number_of_attributes;
		int number_of_classes;
		double** expected_output;
		int classes_min;	//smaller value for classes
		int classes_max; 	//biggest value for classes
		int class_index;

		void train(Reinforcement_Agent* agent);
		void test(Reinforcement_Agent* agent);
		void start(int &number_of_observation_vars, int& number_of_action_vars);
		double step(double* action);
		double restart();
		void print();

		//auxiliary functions
		int numberOfOutputs(double** dataset, int class_index, int number_of_samples);
		void shuffleTrainingDataset();

private:
		//the following function should only be called before the training, 
		//otherwise it is going to mix training and testing datasets
		//For shuffling the training dataset only use shuffleTrainingDataset()
		void shuffleDataset();
};

#endif
