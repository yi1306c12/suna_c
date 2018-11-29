
#include"Copy.h"

Copy::Copy(int input_symbol_possibilities, int number_of_inputs, int sequence_length, Random* random)
{
	trial=-1;
	this->random= random;
	//this->verbose=verbose;

	this->input_symbol_possibilities = input_symbol_possibilities;
	this->number_of_inputs= number_of_inputs;
	this->sequence_length= sequence_length;

	if(input_symbol_possibilities <= 0)
	{
		printf("ERROR: invalid input_symbol_possibilities value, it must be a value greater than 0\n");
		exit(1);
	}

	//allocate the sequence
	input_data= (int**)malloc((sequence_length+1)*sizeof(int*));
	for(int i=0; i<sequence_length;++i)
	{
		input_data[i]= (int*)calloc(number_of_inputs,sizeof(int));
		for(int j=0; j<number_of_inputs ;++j)
		{	
			input_data[i][j]= random->uniform(1,input_symbol_possibilities);
		}
	}
	//delimiter (the last vector after the sequence has finished)
	input_data[sequence_length]= (int*)calloc(number_of_inputs,sizeof(int));
	for(int j=0; j<number_of_inputs ;++j)
	{	
		input_data[sequence_length][j]= 0;
	}


	//max steps is just an arbitrary very big number
	//just to make sure the episode will not stop before it should
	MAX_STEPS= sequence_length*3;

	//printf("all combinations %d\n",all_combinations);

	verbose=false;	
}

Copy::~Copy()
{
	//free(tmp_address);
}

void Copy::start(int &number_of_observation_vars, int& number_of_action_vars)
{
	number_of_observation_vars= number_of_inputs;
	this->number_of_observation_vars= number_of_observation_vars;
	observation= (double*) malloc(number_of_observation_vars*sizeof(double));

	number_of_action_vars=number_of_inputs;
	this->number_of_action_vars=number_of_action_vars;

	// Initialize
	restart();
}

double Copy::step(double* action)
{
	// initial reward
	if(action==NULL)
	{
		return 0;
	}

	//--------------------- Evaluate ------------------------

	double reward= 0.0;

	//check if the sequence has already been presented to the learning algorithm
	//if so, test the algorithm ability to record the previous presented sequence
	if(sequence_counter > sequence_length)
	{
		//the sequence_counter 
		int index= sequence_counter - sequence_length -1;

		//calculate reward
		for(int j=0; j< number_of_inputs ;++j)
		{	
			//notice that mean squared error MSE= 1/n (\sum (y - correct_y)^2)
			double error= action[j]- input_data[index][j]; 
			double squared_error= error*error;
			reward+= -squared_error;
		}

		if(verbose)
		{
			//does not modify the observation
			for(int j=0; j< number_of_inputs ;++j)
			{	
		      		//observation[j]= (double) input_data[sequence_counter][j]; 
				printf("%.0f ",observation[j]);	
			}
			printf("\n");
		}

	}
	else
	{
		//set observation to the sequence
		for(int j=0; j< number_of_inputs ;++j)
		{	
			observation[j]= (double) input_data[sequence_counter][j]; 
	
			if(verbose)
			{		
				printf("%.0f ",observation[j]);	
			}
		}
		if(verbose)
		{
			printf("\n");
		}

		//the reward is left unchanged here
		//since the learning algorithm is just observing for the first time the sequence

	}

	//printf("reward %f \n\n",reward);	

	//--------------------- Check for End of Trial ------------------------
	
	
	sequence_counter++;

	//check if all combinations were already tested
	if(sequence_counter >= sequence_length*2 +2)
	{
		restart();
		//printf("\n");
		return reward;
	}
	
	return reward;
}

bool Copy::set(int feature)
{
	switch(feature)
	{
		case NORMALIZED_OBSERVATION:
		{
			//if it is a binary problem {0,1} so the observation range is always in the normalized range [0,1]
			if(input_symbol_possibilities == 2)
			{
				normalized_observation=true;
				return true;
			}

			//else
			return false;
		}
		break;
		case NORMALIZED_ACTION:
		{
			//if it is a binary problem {0,1} so the action range is always in the normalized range [0,1]
			if(input_symbol_possibilities == 2)
			{
				normalized_action=true;
				return true;
			}

			//else
			return false;
		}
		break;
		default:
		{
			return false;
		}
	}
}

//print all combinations of input data
void Copy::print()
{
	for(int i=0; i<sequence_length ;++i)
	{
		for(int j=0; j<number_of_inputs+1 ;++j)
		{	
			printf("%d ",input_data[i][j]);
		}
		printf("\n");
	}
	
}

double Copy::restart()
{
	trial++;

	//create new sequence
	for(int i=0; i<sequence_length ;++i)
	{
		for(int j=0; j<number_of_inputs ;++j)
		{	
			input_data[i][j]= random->uniform(1,input_symbol_possibilities);
		}
	}

	//reset sequence counter
	sequence_counter=0;	

	//set observation to current combination of inputs
	for(int j=0; j< number_of_inputs ;++j)
	{	
		observation[j]= (double) input_data[sequence_counter][j]; 
	}

	return 0;
}




