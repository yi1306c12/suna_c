
#include"ClassificationKfold.h"

ClassificationKfold::ClassificationKfold(const char* filename, Random* random)
{
	bool has_title=true;
	dataset= readCSV(filename, number_of_samples, number_of_attributes, has_title);
	this->random= random;

	//printMatrix(dataset, number_of_samples, number_of_attributes);

	class_index= number_of_attributes -1;
	//the last attribute is a class and therefore not an attribute
	number_of_attributes= number_of_attributes -1;
	
	//get number of outputs ONLY USE THIS IF CLASS IS NOMINAL (integer value)
	number_of_classes= numberOfOutputs(dataset, class_index, number_of_samples);
		
	//free memory
	for(int i=0;i<number_of_samples;++i)
	{
		free(dataset[i]);
	}
	free(dataset);

	dataset=NULL;
}

ClassificationKfold::~ClassificationKfold()
{

}

void ClassificationKfold::train(Reinforcement_Agent* agent, const char* filename)
{
	int i,repetition;
	
	//get the training dataset and ignore the attributes variable, because it will not change
	int ignored_variable;
	bool has_title=true;
	dataset= readCSV(filename, number_of_samples, ignored_variable, has_title);

#ifdef	BOOSTING
	createModifiedDataset(dataset, number_of_samples, number_of_attributes);
#endif
#ifdef	ADAPTIVE_REWARD
	createModifiedDataset(dataset, number_of_samples, number_of_attributes);
#endif
	
	printf("Training samples %d \n",number_of_samples);

#ifdef	PRINT_SOM		
	char som_filename[FILENAME_SIZE];
	Self_Organizing_Neurons* soc= (Self_Organizing_Neurons*)agent;
#endif

	for(repetition=0;repetition < NUMBER_OF_TRAINING_REPETITIONS ;++repetition)
	{

#ifdef	CLASSIFICATION_VERBOSE
		printf("rep %d\n",repetition);
#endif		
		
		int correct= 0;
		int sum=0;
		double reward=0;

		for(i=0; i<number_of_samples; ++i)
		{

			//set the state of the environment
			//CLASSIFICATION PROBLEM - every time the environment is going to a different sample
#ifdef	BOOSTING
			observation= (double*)memcpy(observation,dataset[resampled_dataset[i]],sizeof(double)*number_of_observation_vars);
#else
			observation= (double*)memcpy(observation,dataset[i],sizeof(double)*number_of_observation_vars);
#endif

			
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
#ifdef	ADAPTIVE_REWARD
				reward=sample_difficulty[i];
#else		
				reward=1;
#endif
			}
			else
			{
				reward=0;
			}

#ifdef	BOOSTING
			updateSampleDifficulty(resampled_dataset[i],reward);
#endif
#ifdef	ADAPTIVE_REWARD
			updateSampleDifficulty(i,reward);
#endif

			sum++;

		}
	
		agent->endEpisode(0);


#ifdef	CLASSIFICATION_VERBOSE
		printf("Training accuracy %f\n",(double)correct/(double)sum);
#endif

		//shuffle the dataset to remove the connection from one action to the other
#ifdef	BOOSTING
		resampleDataset();
#else
		shuffleDataset();
#endif

#ifdef	PRINT_SOM		
		if(i%10==0)
		{
			sprintf(som_filename, "som_map%d",i/10);
			soc->printSOM(som_filename);
		}
#endif		
	}
		

	//free memory
	for(i=0;i<number_of_samples;++i)
	{
		free(dataset[i]);
	}
	free(dataset);
#ifdef	BOOSTING
	free(resampled_dataset);
	free(sample_difficulty);
#endif

	dataset=NULL;
}


double ClassificationKfold::test(Reinforcement_Agent* agent, const char* filename)
{
	int i;
	
	int correct= 0;
	int sum=0;
	//double reward=0;
	
	//get the test dataset and ignore the attributes variable, because it will not change
	int ignored_variable;
	bool has_title=true;
	dataset= readCSV(filename, number_of_samples, ignored_variable, has_title);

	printf("\nTesting the accuracy of the best classifiers\n");

	for(i=0; i<number_of_samples; ++i)
	{
	
		//set the state of the environment
		//CLASSIFICATION PROBLEM - every time the environment is going to a different sample
		observation= (double*)memcpy(observation,dataset[i],sizeof(double)*number_of_observation_vars);
		
		double agent_fitness= agent->stepBestAction(observation);
		
		//telling the compiler that we are not going to use agent_fitness for now
		(void)agent_fitness;

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
			//reward=1;
		}
		else
		{
			//reward=0;
		}

		sum++;

	}

	
	double accuracy= (double)correct/(double)sum;

	printf("Testing samples %d \n",number_of_samples);
	printf("Testing accuracy %f\n",accuracy);

	//free memory
	for(i=0;i<number_of_samples;++i)
	{
		free(dataset[i]);
	}
	free(dataset);
	
	dataset=NULL;

	return accuracy;

}

void ClassificationKfold::createModifiedDataset(double** dataset, int number_of_samples, int number_of_attributes)
{
	int i;

	//alloc the resampled_dataset and make it as a copy of dataset
	resampled_dataset= (int*)malloc(sizeof(int)*number_of_samples);
	for(i=0;i<number_of_samples;++i)
	{
		resampled_dataset[i]= i;
	}

	//alloc sample_difficulty
	sample_difficulty= (int*)malloc(sizeof(int)*number_of_samples);
	for(i=0;i<number_of_samples;++i)
	{
		sample_difficulty[i]=1;
	}
	
}
			
void ClassificationKfold::updateSampleDifficulty(int sample_index, double reward)
{
	//for prediction mistakes (negative or zero rewards)
	//increase the chance of this sample appearing in the dataset (increase sample difficulty)
	if(reward <= 0)
	{
		sample_difficulty[sample_index]++;
	}
	else
	{
		//if the sample_difficulty was already increased and the prediction was correct
		//decrease its chance of appearing
		if(sample_difficulty[sample_index]>1)
		{
	//		sample_difficulty[sample_index]--;
		}
	}
}

void ClassificationKfold::resampleDataset()
{
	int i;
	int sum=0;
	for(i=0;i<number_of_samples;++i)
	{
		sum+= (int) sample_difficulty[i];
	}

	for(int j=0;j<number_of_samples; ++j)
	{
		int random_value= random->uniform(0,sum -1);
		
		int tmp_sum=0;
		bool found= false;
		for(i=0;i<number_of_samples && found == false;++i)
		{
			tmp_sum+= (int) sample_difficulty[i];

			if(random_value < tmp_sum)
			{
				resampled_dataset[j]=i;
				found=true;
			}
		}
	}

	
}

void ClassificationKfold::start(int &number_of_observation_vars, int& number_of_action_vars)
{
	number_of_observation_vars= number_of_attributes;
	this->number_of_observation_vars= number_of_attributes;
	observation= (double*) malloc(number_of_observation_vars*sizeof(double));
	
	number_of_action_vars= number_of_classes;
	this->number_of_action_vars= number_of_classes;

	printf("classes(number of action vars) %d observation_vars %d\n",number_of_classes,number_of_observation_vars);

}

//if outputs are integers, return the number of outputs
int ClassificationKfold::numberOfOutputs(double** dataset, int class_index, int number_of_samples)
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
	}
	
	classes_min= min;
	classes_max= max;
	
	
	return max - min + 1;
}



double ClassificationKfold::step(double* action)
{
	return 0;	
}

double ClassificationKfold::restart()
{
	return 0;	

}

void ClassificationKfold::print()
{

}


void ClassificationKfold::shuffleDataset()
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

