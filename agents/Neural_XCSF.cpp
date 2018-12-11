
#include"Neural_XCSF.h"

Neural_XCSF::Neural_XCSF(int population_size, int number_of_hidden, int number_of_hidden_layers)
{
	random= new State_of_Art_Random(time(NULL));

	this->max_population_size= population_size;
	this->population_size= population_size/10 +1;
	this->number_of_hidden= number_of_hidden;
	this->number_of_hidden_layers= number_of_hidden_layers;
	
	time_stamp= (int*)calloc(max_population_size,sizeof(int));
	match_set= (int*)malloc(max_population_size*sizeof(int));
	previous_match_set= (int*)malloc(max_population_size*sizeof(int));
	previous_action_set= (int*)malloc(max_population_size*sizeof(int));
	pop_match_set_size= (double*)calloc(max_population_size,sizeof(double));
	pop_match_set_size_adjusted_times= (int*)calloc(max_population_size,sizeof(int));

	dna=NULL;
	neuron=NULL;
	action=NULL;
	previous_action_set[0]=-1;
	step_counter=0;
	exploratory= 0;
	trial_counter=1;
	
	//PARAMETERS
	gamma= 0.95;
	beta=0.2;
	epsilon_zero=0.005;
	alpha=0.1;
	v=5.0;
	theta_GA=50;
	x_zero=1.0;
	n=0.2;
	//genetic operator 
	mu=0.2;
	increment=0.05;
	CR=0.1;	//percentage of mutated alleles 
}

Neural_XCSF::~Neural_XCSF()
{
	int i;
	free(action);
		
	//free neurons and dna

	//if allocated
	if(dna != NULL)
	{
		for(i=0;i<max_population_size;++i)
		{
			free(pop_output[i]);
			free(dna[i]);
			delete neuron[i];
		}
	}
	free(pop_output);
	free(dna);
	free(neuron);
	fclose(fp);
}

void Neural_XCSF::init(int number_of_observation_vars, int number_of_action_vars)
{
	int i,j;
	
	this->number_of_observation_vars= number_of_observation_vars;
	this->number_of_action_vars= number_of_action_vars;

	action= (double*) malloc(number_of_action_vars*sizeof(double));
	previous_observation= (double*) malloc(number_of_observation_vars*sizeof(double));


	//set inputs and outputs
	//NOTICE: one output will be the matching output, 
	//then the number of outputs will be greater than the required outputs.
	int number_of_inputs= number_of_observation_vars;
	int number_of_outputs= number_of_action_vars + 1;

	dna_size= (number_of_inputs+number_of_outputs)*number_of_hidden+number_of_hidden*(number_of_hidden_layers-1)*number_of_hidden + number_of_hidden*number_of_hidden_layers;

	//allocate neurons and dna
	neuron= (Simple_Feedforward**)malloc(sizeof(Simple_Feedforward*)*max_population_size);
	dna= (double**)malloc(sizeof(double*)*max_population_size);
	prediction_weights= (double**)malloc(sizeof(double*)*max_population_size);
	for(i=0;i<max_population_size;++i)
	{
		dna[i]= (double*)malloc(sizeof(double)*dna_size);
		neuron[i]= new Simple_Feedforward(number_of_inputs, number_of_hidden, number_of_outputs, number_of_hidden_layers, random);
		prediction_weights[i]= (double*)malloc(sizeof(double)*(number_of_observation_vars+1));
		for(j=0;j<number_of_observation_vars+1;++j)
		{
			prediction_weights[i][j]= random->uniform(0.0,1.0);
		}
		
	}

	//allocate population parameters
	pop_output=(double**)malloc(sizeof(double*)*max_population_size);
	pop_fitness=(double*)malloc(max_population_size*sizeof(double));
	pop_reward_prediction=(double*)calloc(max_population_size, sizeof(double));
	true_reward=(double*)calloc(max_population_size, sizeof(double));
	previous_pop_reward_prediction=(double*)calloc(max_population_size, sizeof(double));
	pop_prediction_error=(double*)calloc(max_population_size,sizeof(double));
	for(i=0;i<max_population_size;++i)
	{
		pop_output[i]=(double*)calloc(number_of_outputs, sizeof(double));
		pop_reward_prediction[i]= 0.0;
		pop_fitness[i]=0.0;
	}

	//initialize neurons and dna
	for(i=0;i<population_size;++i)
	{
		for(j=0;j<dna_size;++j)
		{
			dna[i][j]= random->uniform(-1.0,1.0);
		}
		neuron[i]->setWeights(dna[i]);
	}
}

void Neural_XCSF::step(double* observation, double reward)
{
	int i,j;
	
	//if GA does not happen, the value will stay the same
	new_individual=-1;

	//randomly choose between exploration and exploitation
	if(reward!=0.0)
	{
		exploratory= random->uniform(0,1);
		//exploratory= 0;
		//printf("ex %d\n",exploratory);

		trial_counter++;
	}

	//Reinforcement component
	//reinforcement(reward);
	
//	printf("observation %f\n",observation[0]);

	//define the last output as the "don't match" output
	int match_index= number_of_observation_vars;

	int match_counter=0;

	//insert the observation and receive the neurons' outputs
	for(i=0;i<population_size;++i)
	{
		neuron[i]->activate(observation, pop_output[i]);	
	
//		printInputOutput(observation, pop_output[i]);

//		neuron[i]->print();

//		exit(1);
		
		//insert or not the individual in the Match set [M]
		//(The following is done equally in the article: Towards continuous actions in Continuous space
		//and time using self-adaptive constructivism in neural XCSF)
		if(pop_output[i][match_index] < MATCH_THRESHOLD)
		{
			match_set[match_counter]=i;	
			match_counter++;

//			printf("output %f index %d\n",pop_output[i][match_index], i);
		}
	}
	
	int match_set_size= match_counter;

	//if the match set is empty
	if(match_set_size == 0)
	{
		covering(observation);

		match_set_size=1;
	}
	
	//update estimate of classifier's match set size
	for(i=0;i<match_set_size;++i)
	{
		MAM(pop_match_set_size[match_set[i]], pop_match_set_size_adjusted_times[match_set[i]], beta, match_set_size);
	}

	//compute the reward prediction
	for(i=0;i<match_set_size;++i)
	{
		int individual= match_set[i];

		pop_reward_prediction[individual]= x_zero*prediction_weights[individual][number_of_observation_vars];
		for(j=0;j<number_of_observation_vars;++j)
		{
			pop_reward_prediction[individual]+= observation[j]*prediction_weights[individual][j];
		}
		true_reward[individual]= pop_reward_prediction[individual]*pop_fitness[individual];
		//true_reward[individual]= pop_reward_prediction[individual]*(1.0-pop_prediction_error[individual]);
	}

//	printArray(true_reward,population_size);
//	printArray(pop_fitness,population_size);
//	printf("reward_prediction\n");
//	printArray(pop_reward_prediction,population_size);


	//Action set is a random selected individual from the Match set
	//int random_selection= random->uniform(0,match_set_size-1);
	int random_selection;
	if(exploratory)
	{
		random_selection= random->uniform(0,match_set_size-1);
		//random_selection= indirectRouletteSelection(match_set,match_set_size,true_reward,population_size,random);
		//printf("p %d\n",match_set_size);
		//random_selection= indirectRouletteSelection(match_set,match_set_size,true_reward,population_size,random);
	}
	else
	{
		//random_selection= indirectRouletteSelection(match_set,match_set_size,true_reward,population_size,random);
		random_selection= indirectTournament(match_set, match_set_size, true_reward, TOURNAMENT_SIZE, random);
		//random_selection= indirectTournament(match_set, match_set_size, pop_fitness, TOURNAMENT_SIZE, random);
	}
	
	//Reinforcement component
	reinforcement(true_reward[random_selection]*gamma + reward);

//	printf("action individual %d\n",random_selection);

	//output the random selected individual's output
	for(i=0;i<number_of_action_vars;++i)
	{
		action[i]= pop_output[match_set[random_selection]][i];
	}
	
	int action_set[1];
	action_set[0]= match_set[random_selection];
	int action_set_size=1;

	//call the genetic algorithm over a set of individuals
	//geneticAlgorithm(match_set, match_set_size);
//	geneticAlgorithm(match_set, match_set_size);
//	if(reward!=0.0)
	if(!exploratory)
	{
		//geneticAlgorithm(match_set, match_set_size);
		geneticAlgorithm(action_set, action_set_size);
		//increment steps
		step_counter++;
	}
	
	
	//save current information to the next step
	update(action_set, action_set_size, observation, match_set_size);

	//printInputOutput(observation, action);
	printToFile(match_set_size);
}


//reinforcement for only one individual active
void Neural_XCSF::reinforcement(double current_reward)
{
	int i,j;

	//if some action happened last iteraction
	//update their fitness
	if(previous_action_set[0] != -1)
	{
		for(i=0;i<previous_action_set_size;++i)
		//for(i=0;i<previous_match_set_size;++i)
		{
			int individual= previous_action_set[i];
//			printf("individual %d\n", individual);
			//int individual= previous_match_set[i];

			//update prediction error 
//			printf("current reward %f previous prediction %f\n", current_reward, previous_pop_reward_prediction);
			Widrow_Hoff(pop_prediction_error[individual], beta, fabs(current_reward - previous_pop_reward_prediction[individual]));

			//compute previous observation's module
			double module= x_zero*x_zero;
			for(j=0;j<number_of_observation_vars;++j)
			{
				module+= previous_observation[j]*previous_observation[j];
			}
			module= sqrt(module);
			//update reward prediction (XCSF prediction rule)
			double reward_diff= current_reward - previous_pop_reward_prediction[individual];
			for(j=0;j<number_of_observation_vars;++j)
			{
				double delta_weight= (n/module)*(reward_diff)*previous_observation[j];
				prediction_weights[individual][j]+= delta_weight;
			}
			prediction_weights[individual][number_of_observation_vars]+= (n/module)*(reward_diff)*x_zero;
			//update reward prediction (Simple update for XCS)
			//Widrow_Hoff(pop_reward_prediction[individual], beta, current_reward);

			//update accuracy
			double k;
			if(pop_prediction_error[individual] > epsilon_zero)
			{
				//printf("fitness %f %f %f\n", pop_prediction_error[individual], pop_prediction_error[individual]/epsilon_zero, pow(pop_prediction_error[individual]/epsilon_zero, -v));
				k= alpha*pow(pop_prediction_error[individual]/epsilon_zero, -v);
				//k= exp(log(alpha)*((pop_prediction_error[individual] - epsilon_zero)/epsilon_zero));
			}
			else
			{
				k= 1.0;
			}
			
//			printf("fitness %f %f\n", pop_fitness[individual],k);
			//update Fitness
			Widrow_Hoff(pop_fitness[individual], beta, k);
//			printf("fitness %f\n", pop_fitness[individual]);
		}
		
	}

}

//pass a set of indexes to the individuals
void Neural_XCSF::geneticAlgorithm(int* set, int set_size)
{
	int i;

	//compute the average time_stamp
	double avg_time_stamp=0.0;
	for(i=0;i<set_size;++i)
	{
		avg_time_stamp+= time_stamp[set[i]];
	}
	avg_time_stamp= avg_time_stamp/(double)set_size;

	//check if the genetic algorithm should be run
	if(step_counter - avg_time_stamp > theta_GA)
	{
		//printf("!!!!!!\n");
		
		//int champion= indirectTournament(set, set_size, pop_fitness, TOURNAMENT_SIZE, random);
		//int champion= indirectTournament(set, set_size, true_reward, TOURNAMENT_SIZE, random);

		//execute GA
		for(i=0;i<set_size;++i)
		{
			int selected= set[i];
			//int selected= champion;

			//create new individual
			//int new_individual;
			
			//if there is space for new individuals
			if(population_size < max_population_size)
			{
				//initialize neurons and dna
				new_individual=population_size;

				population_size++;
			}
			else
			{
				//select the individual to be deleted with the biggest size of  
				//average match sets it participates
				//new_individual= inverseRouletteSelection(pop_fitness, population_size, random);
				//new_individual= inverseTournament(pop_fitness,population_size, TOURNAMENT_SIZE, random);
				
				//select worst individual
				new_individual= 0;
				for(i=1;i<population_size;++i)
				{
					if(pop_fitness[i] < pop_fitness[new_individual])
					{
						new_individual= i;
					}
				}



			//	new_individual= inverseTournament(pop_reward_prediction, population_size, TOURNAMENT_SIZE, random);
			//	new_individual= rouletteSelection(pop_match_set_size, population_size, random);
				//new_individual= inverseRouletteSelection(true_reward, population_size, random);
				//new_individual= inverseRouletteSelection(pop_reward_prediction, population_size, random);
				
			}
	
			//copy parent to the new_individual
			memcpy(dna[new_individual], dna[selected], sizeof(double)*dna_size);
			
			//modify new individual
			//incrementalMutation(dna[new_individual], dna_size, mu, increment, random);
			incrementalGaussianMutation(dna[new_individual], dna_size, mu, random);
			
			//apply differential evolution
			//double F= 0.5;
			//double F= random->uniform(0.0,1.0);
			//differentialEvolutionOperator(dna[new_individual], CR, F, selected, dna_size, dna, population_size, random);
			//indirectDifferentialEvolutionOperator(dna[new_individual], CR, F, selected, dna_size, dna, set, set_size, random);
			
			neuron[new_individual]->setWeights(dna[new_individual]);
		
			//copy variables from the parent
			pop_reward_prediction[new_individual]= pop_reward_prediction[selected];
			//pop_reward_prediction[new_individual = 0;
			pop_prediction_error[new_individual]= pop_prediction_error[selected];
			pop_fitness[new_individual]= pop_fitness[selected];
			memcpy(prediction_weights[new_individual], prediction_weights[selected],sizeof(double)*(number_of_observation_vars));

			//update time_stamp of the individuals which participated in the GA
			time_stamp[new_individual]= step_counter;
			time_stamp[selected]= step_counter;
		}
	
		//for(i=0;i<set_size;++i)
		//{
		//	time_stamp[set[i]]= step_counter;
		//}

	}
		

		
	//increment steps
	//step_counter++;
	
}

void Neural_XCSF::update(int* current_action_set, int action_set_size, double* observation, int match_set_size)
{
	int i;

	//update the previous observation with the current one
	memcpy(previous_observation,observation,sizeof(double)*number_of_observation_vars);
	memcpy(previous_pop_reward_prediction,pop_reward_prediction,sizeof(double)*max_population_size);
	memcpy(previous_match_set,match_set,sizeof(int)*max_population_size);
	previous_match_set_size= match_set_size;
	
	//update the previous action set with the current action set
	for(i=0;i<action_set_size;++i)
	{
		previous_action_set[i]= current_action_set[i];
	}
	previous_action_set_size= action_set_size;
}

void Neural_XCSF::print()
{
	int i;

	printf("true_reward reward_prediction prediction_error fitness\n");
	for(i=0;i<population_size;++i)
	{
		printf("%f %f %f %f\n",true_reward[i], pop_reward_prediction[i], pop_prediction_error[i],pop_fitness[i]);
	}
	printf("previous action %d\n",previous_action_set[0]);
}

//insert classifiers which "match" the given observation
//if the population_size was exceeded, remove a classifier
void Neural_XCSF::covering(double* observation)
{
	int i,j;

	//printf("covering\n");
	
	//define the last output as the "don't match" output
	int match_index= number_of_observation_vars;
	
	//if there is space for new individuals
	if(population_size < max_population_size)
	{
		//initialize neurons and dna
		i=population_size;

		//just insert a "don't match" value, to make it enter in the while loop!
		pop_output[i][match_index]=1.0;
		
		//repeat the creation process while the neuron "don't match"
		while(pop_output[i][match_index] > MATCH_THRESHOLD)
		{
			for(j=0;j<dna_size;++j)
			{
				dna[i][j]= random->uniform(-1.0,1.0);
			}
			neuron[i]->setWeights(dna[i]);
			
			neuron[i]->activate(observation, pop_output[i]);	

		}
			
		population_size++;
		match_set[0]=i;	
	}
	else
	{
		//select the individual to be deleted with the biggest size of  
		//average match sets it participates
		i= rouletteSelection(pop_match_set_size, population_size, random);
		
		//just insert a "don't match" value, to make it enter in the while loop!
		pop_output[i][match_index]=1.0;
		
		//repeat the creation process while the neuron "don't match"
		while(pop_output[i][match_index] > MATCH_THRESHOLD)
		{
			for(j=0;j<dna_size;++j)
			{
				dna[i][j]= random->uniform(-1.0,1.0);
			}
			neuron[i]->setWeights(dna[i]);
			
			neuron[i]->activate(observation, pop_output[i]);	

		}
			
		match_set[0]=i;	
	}
}

void Neural_XCSF::printToFile(int match_set_size)
{
	if(fp==NULL)
	{
		fp= fopen("details","w");
		fprintf(fp,"trial_counter step_counter match_set_size action_individual removed(new)_individual\n");
	}

	fprintf(fp,"%d %d %d %d %d %d\n",trial_counter, step_counter, match_set_size, previous_action_set[0], population_size, new_individual);
}


void Neural_XCSF::printInputOutput(double* observation, double* action)
{
	printf("Observation\n");
	printArray(observation, number_of_observation_vars);
	printf("Action\n");
	printArray(action, number_of_action_vars);
}
		
void Neural_XCSF::endEpisode()
{

}
		
double Neural_XCSF::stepBestAction(double* observation)
{
	return 0;
}

void Neural_XCSF::saveAgent(const char* filename)
{
	printf("saveAgent() not implemented\n");

}

void Neural_XCSF::loadAgent(const char* filename)
{
	printf("loadAgent() not implemented\n");
}

