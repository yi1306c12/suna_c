
#include"Classification.h"

Classification::Classification(const char* filename)
{
	bool has_title=true;
	dataset= readCSV(filename, number_of_samples, number_of_attributes, has_title);

	//printMatrix(dataset, number_of_samples, number_of_attributes);

	class_index= number_of_attributes -1;
	//the last attribute is a class and therefore not an attribute
	number_of_attributes= number_of_attributes -1;
	
	shuffleDataset();
}

Classification::~Classification()
{

}

void Classification::train(Reinforcement_Agent* agent)
{
	int i;


	int correct= 0;
	int sum=0;
	double reward=0;

	for(i=0; i<(int)((double)(number_of_samples)*RATIO_OF_SAMPLES_FOR_TRAINING); ++i)
	{

		//set the state of the environment
		//CLASSIFICATION PROBLEM - every time the environment is going to a different sample
		observation= (double*)memcpy(observation,dataset[i],sizeof(double)*number_of_observation_vars);
		
		agent->step(observation, reward);

		//printArray(observation,number_of_observation_vars);
		//printArray(agent->action,number_of_action_vars);


		//RSS+= euclidian_distance(correct_responses[i],response, neural_network->number_of_outputs);
			
		//check if the output's answer (index which has maximum value) is the correct_class
		int max_index_response=0;
		int correct_class= dataset[i][class_index] - classes_min;
		//printf("correct class= %d\n",correct_class);
		//exit(1);

		for(int j=0; j< number_of_action_vars; ++j)
		{
			if(agent->action[max_index_response]< agent->action[j])
			{
				max_index_response=j;
			}
		}

		if(max_index_response == correct_class)
		{
			correct++;
			reward=1;
		}
		else
		{
			reward=0;
		}

		sum++;

	}

	printf("Training accuracy %f\n",(double)correct/(double)sum);

}


void Classification::test(Reinforcement_Agent* agent)
{
	int i;
	
	int correct= 0;
	int sum=0;
	double reward=0;

	printf("\nTesting the accuracy of the best classifiers\n");

	for(i=0; i<(int)((double)(number_of_samples)*RATIO_OF_SAMPLES_FOR_TRAINING); ++i)
	{

		//set the state of the environment
		//CLASSIFICATION PROBLEM - every time the environment is going to a different sample
		observation= (double*)memcpy(observation,dataset[i],sizeof(double)*number_of_observation_vars);
		
		double agent_fitness= agent->stepBestAction(observation);

		//printArray(observation,number_of_observation_vars);


		//RSS+= euclidian_distance(correct_responses[i],response, neural_network->number_of_outputs);
			
		//check if the output's answer (index which has maximum value) is the correct_class
		int max_index_response=0;
		int correct_class= dataset[i][class_index] - classes_min;

		for(int j=0; j< number_of_action_vars; ++j)
		{
			if(agent->action[max_index_response]< agent->action[j])
			{
				max_index_response=j;
			}
		}

		if(max_index_response == correct_class)
		{
			correct++;
			reward=1;
		}
		else
		{
			reward=0;
		}

		sum++;

	}

	printf("Training accuracy %f\n",(double)correct/(double)sum);


	correct= 0;
	sum=0;
	reward=0;

	for(i=(int)((double)(number_of_samples)*RATIO_OF_SAMPLES_FOR_TRAINING); i<number_of_samples; ++i)
	{
	
		//set the state of the environment
		//CLASSIFICATION PROBLEM - every time the environment is going to a different sample
		observation= (double*)memcpy(observation,dataset[i],sizeof(double)*number_of_observation_vars);
		
		double agent_fitness= agent->stepBestAction(observation);


		//RSS+= euclidian_distance(correct_responses[i],response, neural_network->number_of_outputs);
			
		//check if the output's answer (index which has maximum value) is the correct_class
		int max_index_response=0;
		int correct_class= dataset[i][class_index] - classes_min;

		for(int j=0; j< number_of_action_vars; ++j)
		{
			if(agent->action[max_index_response]< agent->action[j])
			{
				max_index_response=j;
			}
		}

		if(max_index_response == correct_class)
		{
			correct++;
			reward=1;
		}
		else
		{
			reward=0;
		}

		sum++;

	}

	printf("Testing accuracy %f\n",(double)correct/(double)sum);


}

void Classification::start(int &number_of_observation_vars, int& number_of_action_vars)
{
	number_of_observation_vars= number_of_attributes;
	this->number_of_observation_vars= number_of_attributes;
	observation= (double*) malloc(number_of_observation_vars*sizeof(double));
	
	//get number of outputs ONLY USE THIS IF CLASS IS NOMINAL (integer value)
	number_of_classes= numberOfOutputs(dataset, class_index, number_of_samples);
	
	number_of_action_vars= number_of_classes;
	this->number_of_action_vars= number_of_classes;

	printf("classes(number of action vars) %d observation_vars %d\n",number_of_classes,number_of_observation_vars);

}

//if outputs are integers, return the number of outputs
int Classification::numberOfOutputs(double** dataset, int class_index, int number_of_samples)
{
	int i;

	int min=dataset[0][class_index];
	int max=dataset[0][class_index];
	for(i=1;i<number_of_samples;++i)
	{
		if(min > dataset[i][class_index])
		{
			min=dataset[i][class_index];
		}
		if(max < dataset[i][class_index])
		{
			max=dataset[i][class_index];
		}
		//printf("%.2f\n",expected_output[0][i]);
	}
	
	classes_min= min;
	classes_max= max;
	
	
	return max - min + 1;
}



double Classification::step(double* action)
{
	return 0;	
}

double Classification::restart()
{
	return 0;	

}

void Classification::print()
{

}


void Classification::shuffleDataset()
{
	int i;

	//the number of variables in the dataset are the number_of_attributes plus the class variable
	int number_of_variables= number_of_attributes +1;

	double * tmp_mem= (double*)malloc(sizeof(double)*number_of_variables);

	int number_of_swaps= number_of_samples/2;	

	//shuffle dataset
	for(i=0;i<number_of_swaps;i++)
	{
		//swap two instance positions
		int first= rand()%number_of_samples;
		int second= rand()%number_of_samples;

		//tmp_mem = first
		memcpy(tmp_mem, dataset[first], sizeof(double)*number_of_variables);
		//first = second
		memcpy(dataset[first], dataset[second], sizeof(double)*number_of_variables);
		//second = tmp_mem
		memcpy(dataset[second], tmp_mem, sizeof(double)*number_of_variables);
	}

	free(tmp_mem);
}

void Classification::shuffleTrainingDataset()
{
	int i;

	//the number of variables in the dataset are the number_of_attributes plus the class variable
	int number_of_variables= number_of_attributes +1;

	double * tmp_mem= (double*)malloc(sizeof(double)*number_of_variables);

	int number_of_training_samples= ((double)number_of_samples)*RATIO_OF_SAMPLES_FOR_TRAINING;

	int number_of_swaps= number_of_training_samples/2;	

	//shuffle dataset
	for(i=0;i<number_of_swaps;i++)
	{
		//swap two instance positions
		int first= rand()%number_of_training_samples;
		int second= rand()%number_of_training_samples;

		//tmp_mem = first
		memcpy(tmp_mem, dataset[first], sizeof(double)*number_of_variables);
		//first = second
		memcpy(dataset[first], dataset[second], sizeof(double)*number_of_variables);
		//second = tmp_mem
		memcpy(dataset[second], tmp_mem, sizeof(double)*number_of_variables);
	}

	free(tmp_mem);
}

