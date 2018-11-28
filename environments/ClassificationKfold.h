
#ifndef CLASSIFICATION_H
#define CLASSIFICATION_H

#include"stdlib.h"
#include"stdio.h"
#include"string.h"
#include"useful/read_csv.h"
#include"useful/useful_utils.h"
#include"../agents/Reinforcement_Agent.h"
#include"../agents/Self_Organizing_Neurons.h"
#include"Reinforcement_Environment.h"
#include"random/State_of_Art_Random.h"
#include"../parameters.h"

class ClassificationKfold : public Reinforcement_Environment
{
	public:

		ClassificationKfold(const char* filename, Random* random);
		~ClassificationKfold();

		//All Reinforcement Problems have the observartion variable, although it is not declared here!
		//double* observation;		
		//int number_of_observation_vars;
		//int number_of_action_vars;
		//int trial;
		
		double** dataset;
		int number_of_samples;
		int number_of_attributes;
		int number_of_classes;
		int classes_min;	//smaller value for classes
		int classes_max; 	//biggest value for classes
		int class_index;
		Random* random;
		int* resampled_dataset;
		int* sample_difficulty;

		void train(Reinforcement_Agent* agent, const char* filename);
		double test(Reinforcement_Agent* agent, const char* filename);
		void start(int &number_of_observation_vars, int& number_of_action_vars);
		double step(double* action);
		double restart();
		void print();

		//auxiliary functions
		int numberOfOutputs(double** dataset, int class_index, int number_of_samples);
		void createModifiedDataset(double** dataset, int number_of_samples, int number_of_attributes);
		void resampleDataset();
		void updateSampleDifficulty(int sample_index, double reward);

private:
		//the following function should only be called before the training, 
		//otherwise it is going to mix training and testing datasets
		//For shuffling the training dataset only use shuffleTrainingDataset()
		void shuffleDataset();
};

#endif
