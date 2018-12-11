
#include"XCSF.h"

XCSF::XCSF(int population_size)
{
	random= new State_of_Art_Random(time(NULL));

	this->max_population_size= population_size;
	this->population_size= population_size/10 +1;
	
	time_stamp= (int*)calloc(max_population_size,sizeof(int));
	numerosity= (int*)calloc(max_population_size,sizeof(int));
	match_set= (int*)malloc(max_population_size*sizeof(int));
	action_set= (int*)malloc(max_population_size*sizeof(int));
	previous_match_set= (int*)malloc(max_population_size*sizeof(int));
	previous_action_set= (int*)malloc(max_population_size*sizeof(int));
	k= (double*)calloc(max_population_size,sizeof(double));
	deletion_vote= (double*)calloc(max_population_size,sizeof(double));
	pop_match_set_size= (double*)calloc(max_population_size,sizeof(double));
	pop_match_set_size_adjusted_times= (int*)calloc(max_population_size,sizeof(int));
	pop_experience= (int*)calloc(max_population_size,sizeof(int));

	exceeding_numerosity=0;
	dna=NULL;
	action=NULL;
	previous_action_set[0]=-1;
	step_counter=0;
	exploratory= 0;
	trial_counter=1;
	
	//PARAMETERS
	gamma= 0.95;
	delta= 0.1;
	beta=0.2;
	epsilon_zero=0.1;
	alpha=0.1;
	v=5.0;
	theta_GA=100;
	theta_del=50;
	theta_sub=50;
	x_zero=1.0;
	n=0.2;
	//genetic operator 
	mu=0.2;
	increment=0.05;
	CR=0.1;	//percentage of mutated alleles 
}

XCSF::~XCSF()
{
	int i;
	free(action);
		
	//free neurons and dna

	//if allocated
	if(dna != NULL)
	{
		for(i=0;i<max_population_size;++i)
		{
			free(dna[i]);
		}
	}
	free(dna);
	fclose(fp);
}

void XCSF::init(int number_of_observation_vars, int number_of_action_vars)
{
	int i,j;
	
	this->number_of_observation_vars= number_of_observation_vars;
	this->number_of_action_vars= number_of_action_vars;
	number_of_states= pow(2,number_of_action_vars);

	action= (double*) malloc(number_of_action_vars*sizeof(double));
	previous_observation= (double*) malloc(number_of_observation_vars*sizeof(double));


	//set inputs and outputs
	//NOTICE: one output will be the matching output, 
	//then the number of outputs will be greater than the required outputs.
	int number_of_inputs= number_of_observation_vars;
	int number_of_outputs= number_of_action_vars;

	dna_size= 2*number_of_inputs /*condition*/ + number_of_outputs /*action*/; 

	//allocate dna
	dna= (double**)malloc(sizeof(double*)*max_population_size);
	prediction_weights= (double**)malloc(sizeof(double*)*max_population_size);
	for(i=0;i<max_population_size;++i)
	{
		dna[i]= (double*)malloc(sizeof(double)*dna_size);
		prediction_weights[i]= (double*)malloc(sizeof(double)*(number_of_observation_vars+1));
		for(j=0;j<number_of_observation_vars+1;++j)
		{
			prediction_weights[i][j]= random->uniform(0.0,1.0);
		}
		
	}

	//allocate population parameters
	pop_fitness=(double*)malloc(max_population_size*sizeof(double));
	pop_reward_prediction=(double*)calloc(max_population_size, sizeof(double));
	true_reward=(double*)calloc(max_population_size, sizeof(double));
	previous_pop_reward_prediction=(double*)calloc(max_population_size, sizeof(double));
	pop_prediction_error=(double*)calloc(max_population_size,sizeof(double));
	tmp_individual= (double*)malloc(sizeof(double)*dna_size);
	for(i=0;i<max_population_size;++i)
	{
		pop_reward_prediction[i]= 0.0;
		pop_fitness[i]=0.0;
		numerosity[i]=1;
	}

	//initialize dna
	for(i=0;i<population_size;++i)
	{
		for(j=0;j<dna_size;++j)
		{
			dna[i][j]= random->uniform(-1.0,1.0);
		}
	}
}

void XCSF::step(double* observation, double reward)
{
	int i,j;
	
	//if GA does not happen, the value will stay the same
	new_individual=-1;

	//randomly choose between exploration and exploitation
	if(reward!=-0.5)
	{
		exploratory= random->uniform(0,1);
		//exploratory= 0;
		//printf("ex %d\n",exploratory);

		trial_counter++;
	}

	//Reinforcement component
	//reinforcement(reward);
	
	int match_counter=0;

	//insert the observation and receive the neurons' outputs
	for(i=0;i<population_size;++i)
	{
//		printInputOutput(observation, pop_output[i]);

		bool match=true;
		for(j=0; j<number_of_observation_vars&&match; ++j)
		{
			//dna[2*j]   - minimum of variable j
			//dna[2*j+1] - maximum of variable j
			if(observation[j] < dna[i][2*j] || observation[j] > dna[i][2*j+1])
			{
				match=false;
			}

		}

		//insert or not the individual in the Match set [M]
		if(match)
		{
			match_set[match_counter]=i;	
			match_counter++;
		}
	}
	
	match_set_size= match_counter;

	/*
	//if the match set is empty
	if(match_set_size == 0)
	{
		covering(observation);

		match_set_size=1;
	}
	*/

	/*
	printArray(observation,number_of_observation_vars);
	
	for(i=0;i<match_set_size;++i)
	{
		
		int tmp=i;
		i=match_set[i];

		for(j=0;j<dna_size;++j)
		{
			printf("%f ",dna[i][j]);
		}

		printf("\t %f %f %f %f\n",true_reward[i], pop_reward_prediction[i], pop_prediction_error[i],pop_fitness[i]);
		i=tmp;
	}

	coveringForEveryAction(observation);

	printf("after covering\n");
	
	//update estimate of classifier's match set size
	for(i=0;i<match_set_size;++i)
	{
		MAM(pop_match_set_size[match_set[i]], pop_match_set_size_adjusted_times[match_set[i]], beta, match_set_size);
		
		int tmp=i;
		i=match_set[i];

		for(j=0;j<dna_size;++j)
		{
			printf("%f ",dna[i][j]);
		}

		printf("\t %f %f %f %f\n",true_reward[i], pop_reward_prediction[i], pop_prediction_error[i],pop_fitness[i]);
		i=tmp;
	}

	exit(1);
	*/

	coveringForEveryAction(observation);
	
	//update the action size estimate for every individual present in the match set
	for(i=0;i<match_set_size;++i)
	{
		MAM(pop_match_set_size[match_set[i]], pop_match_set_size_adjusted_times[match_set[i]], beta, match_set_size);
	}

	double best_action_payoff;
	double best_action=-1;
	
//	if(!exploratory)
//	{
		int action_state;
		for(action_state=0; action_state < number_of_states; ++action_state)
		{
			double sum_fitness=0;
			double action_payoff=0;

			for(i=0;i<match_set_size;++i)
			{
				int individual= match_set[i];
				
				int individual_action= getIntegerOutput(dna[individual]);

				if(individual_action == action_state)
				{
					//compute the reward prediction
					pop_reward_prediction[individual]= x_zero*prediction_weights[individual][number_of_observation_vars];
					for(j=0;j<number_of_observation_vars;++j)
					{
						pop_reward_prediction[individual]+= observation[j]*prediction_weights[individual][j];
					}
					
					action_payoff+= pop_reward_prediction[individual]*(pop_fitness[individual]+0.001);
					//true_reward[individual]= pop_reward_prediction[individual]*(1.0-pop_prediction_error[individual]);	
					sum_fitness+= pop_fitness[individual]+0.001;
				}
			}

			//if there is any action in the match set
			if(sum_fitness > 0.0)
			{
				action_payoff/= sum_fitness;

				//check if this action is better than the best one until now
				if(best_action==-1 || best_action_payoff < action_payoff)
				{
					best_action_payoff= action_payoff;
					best_action= action_state;
				}
			}
		}

		//printf("best payoff %f\n",best_action_payoff);
/*	}
	else
	{
		best_action= random->uniform(0,number_of_states-1);
	}
*/	

//	printArray(true_reward,population_size);
//	printArray(pop_fitness,population_size);
//	printf("reward_prediction\n");
//	printArray(pop_reward_prediction,population_size);

	/*
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
	*/
	
	//Reinforcement component
	reinforcement(best_action_payoff*gamma + reward);

	//create the action set from match set
	int action_counter=0;
	for(i=0;i<match_set_size;++i)
	{
		int individual= match_set[i];
		
		int individual_action= getIntegerOutput(dna[individual]);

		//if the individual has the best action, 
		//include him also in the action set
		if(individual_action == best_action)
		{
			action_set[action_counter]= individual;
			action_counter++;
		}
	}
	
	action_set_size= action_counter;

	//printf("individual %d\n",action_set[0]);

	//output the selected output
	int individual= action_set[0];
	int offset= number_of_observation_vars*2;
	for(i=0;i<number_of_action_vars;++i)
	{
		if(dna[individual][offset + i] > 0.0)
		{
			action[i]= 1.0;
		}
		else
		{
			action[i]= -1.0;
		}
	}
	

	//call the genetic algorithm over a set of individuals
	//geneticAlgorithm(match_set, match_set_size);
//	geneticAlgorithm(match_set, match_set_size);
//	if(reward!=0.0)
//	if(exploratory)
	{
		geneticAlgorithm(match_set, match_set_size);
		//geneticAlgorithm(action_set, action_set_size);
		//increment steps
		step_counter++;
	}
	
	
	//save current information to the next step
	update(action_set, action_set_size, observation, match_set_size);

	//printInputOutput(observation, action);
	printToFile(match_set_size);
}


//reinforcement for only one individual active
void XCSF::reinforcement(double current_reward)
{
	int i,j;

	//if some action happened last iteraction
	//update their fitness
	if(previous_action_set[0] != -1)
	{
		double accuracy_sum=0.0;

		for(i=0;i<previous_action_set_size;++i)
		{
			int individual= previous_action_set[i];
			
			//individual excluded in last interaction
			if(individual>=population_size)
			{
				continue;
			}

			//int individual= previous_match_set[i];

			//increase experience
			pop_experience[individual]++;

			//update prediction error 
			//printf("current reward %f previous prediction %f\n", current_reward, previous_pop_reward_prediction[individual]);
			Widrow_Hoff(pop_prediction_error[individual], beta, fabs(current_reward - previous_pop_reward_prediction[individual]));

			//compute previous observation's module
			double module= x_zero*x_zero;
			for(j=0;j<number_of_observation_vars;++j)
			{
				module+= previous_observation[j]*previous_observation[j];
			}
			//module= (module);
			//update reward prediction (XCSF prediction rule)
			double reward_diff= current_reward - previous_pop_reward_prediction[individual];
			//printf("diff %f\n",reward_diff);
			for(j=0;j<number_of_observation_vars;++j)
			{
				double delta_weight= (n/module)*(reward_diff)*previous_observation[j];
				prediction_weights[individual][j]+= delta_weight;
				//printf("delta %f\n",delta_weight);
			}
			prediction_weights[individual][number_of_observation_vars]+= (n/module)*(reward_diff)*x_zero;
		//	printf("delta %f\n", (n/module)*(reward_diff)*x_zero);
			//printf("delta %f\n",delta_weight);
			//update reward prediction (Simple update for XCS)
			//Widrow_Hoff(pop_reward_prediction[individual], beta, current_reward);

			//update accuracy
			if(pop_prediction_error[individual] > epsilon_zero)
			{
				//printf("fitness %f %f %f\n", pop_prediction_error[individual], pop_prediction_error[individual]/epsilon_zero, pow(pop_prediction_error[individual]/epsilon_zero, -v));
				k[individual]= alpha*pow(pop_prediction_error[individual]/epsilon_zero, -v);
				//k= exp(log(alpha)*((pop_prediction_error[individual] - epsilon_zero)/epsilon_zero));
			}
			else
			{
				k[individual]= 1.0;
			}

			//printf("k %f\n",k[individual]);
			
			accuracy_sum+= k[individual]*numerosity[individual];
		}
		
		//update Fitness
		for(i=0;i<previous_action_set_size;++i)
		{
			int individual= previous_action_set[i];
			
			Widrow_Hoff(pop_fitness[individual], beta, k[individual]*(double)numerosity[individual]/accuracy_sum);
			//printf("fitness %f\n", k[individual]*(double)numerosity[individual]/accuracy_sum);
		}
		
	}

}

//pass a set of indexes to the individuals
void XCSF::geneticAlgorithm(int* set, int set_size)
{
	int i,k;

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
		for(k=0;k<2;++k)
		{
			int champion= indirectTournament(set, set_size, pop_fitness, TOURNAMENT_SIZE, random);
			//int champion= indirectTournament(set, set_size, true_reward, TOURNAMENT_SIZE, random);

			//execute GA
			//for(i=0;i<set_size;++i)
			{
				//int selected= set[i];
				int selected= champion;
				

				
				/*
				//if there is space for new individuals
				if(population_size < max_population_size)
				{
					//initialize neurons and dna
					new_individual=population_size;

					population_size++;
				}
				else
				{
					//new_individual=champion;
					//be sure that the champion and the new individual are different
					//while(champion==new_individual)
					{
						//select the individual to be deleted with the biggest size of  
						//average match sets it participates
						//new_individual= inverseRouletteSelection(pop_fitness, population_size, random);
						//new_individual= inverseTournament(pop_fitness,population_size, TOURNAMENT_SIZE, random);
						if(set_size==1)
						{
							//select worst individual
							new_individual= 0;
							for(i=1;i<population_size;++i)
							{
								if(pop_fitness[i] < pop_fitness[new_individual])
								{
									new_individual= i;
								}
							}
						}
						else
						{
							
				*/			//select worst individual from the set
							/*new_individual= 0;
							for(i=1;i<set_size;++i)
							{
								int tmp= set[i];
								if(pop_fitness[tmp] < pop_fitness[new_individual])
								{
									new_individual= tmp;
								}
							}
							*/
				/*			
							new_individual= random->uniform(0,population_size-1);
							//new_individual= set[random->uniform(0,set_size-1)];

						}
						
					}
					

				//	new_individual= inverseTournament(pop_reward_prediction, population_size, TOURNAMENT_SIZE, random);
				//	new_individual= rouletteSelection(pop_match_set_size, population_size, random);
					//new_individual= inverseRouletteSelection(true_reward, population_size, random);
					//new_individual= inverseRouletteSelection(pop_reward_prediction, population_size, random);
					
				}
				*/
		
				//copy parent to the new_individual
				memcpy(tmp_individual, dna[selected], sizeof(double)*dna_size);
				
				//modify new individual
				//incrementalMutation(dna[new_individual], dna_size, mu, increment, random);
				//incrementalGaussianMutation(dna[new_individual], dna_size, mu, random);
				
				//apply differential evolution
				//double F= 0.2;
				double F= random->uniform(0.0,1.0);
				differentialEvolutionOperator(tmp_individual, CR, F, selected, dna_size, dna, population_size, random);
				//indirectDifferentialEvolutionOperator(dna[new_individual], CR, F, selected, dna_size, dna, set, set_size, random);
				
				bool subsumption=false;

				if(doesSubsume(selected,tmp_individual))
				{
					numerosity[selected]++;
					exceeding_numerosity++;
					subsumption=true;

					deleteOneIndividual();

					continue;
				}

				int k;
				for(k=0;k<population_size;++k)
				{
					if(doesSubsume(k, tmp_individual))
					{
						numerosity[k]++;
						exceeding_numerosity++;
						subsumption=true;

						deleteOneIndividual();
						
						continue;
					}
				}
				

				
				//Check if DELETION is necessary
				// and find where to insert individual
				new_individual= individualForDeletion();

				//insert individual
				memcpy(dna[new_individual],tmp_individual,sizeof(double)*dna_size);

				
				//copy variables from the parent
				pop_reward_prediction[new_individual]= pop_reward_prediction[selected];
				//pop_reward_prediction[new_individual = 0;
				pop_prediction_error[new_individual]= pop_prediction_error[selected];
				pop_fitness[new_individual]= pop_fitness[selected];
				memcpy(prediction_weights[new_individual], prediction_weights[selected],sizeof(double)*(number_of_observation_vars));
				numerosity[new_individual]=1;

				pop_experience[new_individual]=0;

				//update time_stamp of the individuals which participated in the GA
				time_stamp[new_individual]= step_counter;
				time_stamp[selected]= step_counter;
		
			}
		}
	
		for(i=0;i<set_size;++i)
		{
			time_stamp[set[i]]= step_counter;
		}

	}
		

		
	//increment steps
	//step_counter++;
	
}

void XCSF::update(int* current_action_set, int action_set_size, double* observation, int match_set_size)
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

void XCSF::print()
{
	int i,j;

	printf("true_reward reward_prediction prediction_error fitness numerosity\n");
	for(i=0;i<population_size;++i)
	{
		for(j=0;j<dna_size;++j)
		{
			printf("%f ",dna[i][j]);
		}

		printf("\t %f %f %f %f %d\n",true_reward[i], pop_reward_prediction[i], pop_prediction_error[i],pop_fitness[i], numerosity[i]);
	}
	printf("previous action %d\n\n",previous_action_set[0]);
	

	int true_exceed=0;
	for(i=0;i<population_size;++i)
	{
		if(numerosity[i]>1)
		{
			true_exceed+= numerosity[i]-1;	
		}
	}


	printf("exceed %d tru exceed %d pop %d \n",exceeding_numerosity, true_exceed, population_size);

}

//insert classifiers which "match" the given observation
//if the population_size was exceeded, remove a classifier
void XCSF::covering(double* observation)
{
	int i,j;
	
	//Check if DELETION is necessary
	// and find where to insert individual
	i= individualForDeletion();

	//guarantee that the created classifier (also called covering classifier)
	//will enter in the match set
	for(j=0; j<number_of_observation_vars; ++j)
	{

		//dna[2*j]   - minimum of variable j
		//dna[2*j+1] - maximum of variable j
		while(observation[j] < dna[i][2*j])
		{
			dna[i][2*j]-= random->uniform(0.0,1.0);
		}
		while(observation[j] > dna[i][2*j+1])
		{
			dna[i][2*j+1]+= random->uniform(0.0,1.0);
		}

	}
		
	match_set[0]= i;	
	resetIndividual(i);
	match_set_size++;
			
}

//	-insert classifiers which "match" the given observation, one for each action
//  	-if the population_size was exceeded, remove a classifier
void XCSF::coveringForEveryAction(double* observation)
{
	int i;
		
	int action_state;
	for(action_state=0; action_state < number_of_states; ++action_state)
	{
		bool action_in_match_set=false;

		//check for a given action in the match set
		for(i=0;i<match_set_size && action_in_match_set==false;++i)
		{
			int individual= match_set[i];
			
			int individual_action= getIntegerOutput(dna[individual]);

			if(individual_action == action_state)
			{
				action_in_match_set=true;
			}
		}

		//if match is not present do covering for this action
		if(action_in_match_set==false)
		{
			coveringForAction(observation, action_state);
		}

	}
		
}

//make a covering classifier with the action passed as parameter and insert it in the population
void XCSF::coveringForAction(double* observation, int action_state)
{
	int i,j;

	//Check if DELETION is necessary
	// and find where to insert individual
	i= individualForDeletion();


	//guarantee that the created classifier (also called covering classifier)
	//will enter in the match set
	for(j=0; j<number_of_observation_vars; ++j)
	{
		dna[i][2*j]= random->uniform(-1.0,1.0);
		dna[i][2*j+1]= random->uniform(-1.0,1.0);

		//dna[2*j]   - minimum of variable j
		//dna[2*j+1] - maximum of variable j
		while(observation[j] < dna[i][2*j])
		{
			dna[i][2*j]-= random->uniform(0.0,1.0);
		}
		while(observation[j] > dna[i][2*j+1])
		{
			dna[i][2*j+1]+= random->uniform(0.0,1.0);
		}

	}

	//make the covering classifier have the action passed as parameter
	for(j=0; j<number_of_action_vars ;++j)
	{
		if(action_state > 0)
		{
			if(action_state%2==0)
			{
				dna[i][j+2*number_of_observation_vars]= random->uniform(-1.0,-0.5);
			}
			else
			{
				dna[i][j+2*number_of_observation_vars]= random->uniform(0.5,1.0);
			}

			action_state/=2;
		}
		else
		{
			dna[i][j+2*number_of_observation_vars]= random->uniform(-1.0,-0.5);
		}
	}

	//insert covering classifier in the match set
	match_set[match_set_size]=i;	
	resetIndividual(i);
	match_set_size++;
}

void XCSF::printToFile(int match_set_size)
{
	if(fp==NULL)
	{
		fp= fopen("details","w");
		fprintf(fp,"trial_counter step_counter match_set_size action_individual removed(new)_individual\n");
	}

	fprintf(fp,"%d %d %d %d %d %d\n",trial_counter, step_counter, match_set_size, previous_action_set[0], population_size, new_individual);
}


void XCSF::printInputOutput(double* observation, double* action)
{
	printf("Observation\n");
	printArray(observation, number_of_observation_vars);
	printf("Action\n");
	printArray(action, number_of_action_vars);
}

int XCSF::getIntegerOutput(double* dna)
{
	int i;

	//begin at the dna position where the actions are stored
	int offset= number_of_observation_vars*2;

	int output=0;

	for(i=0;i<number_of_action_vars;++i)
	{
		if(dna[offset + i] > 0.0)
		{
			output+= pow(2,i);
		}
	}

	return output;
}

//Check if DELETION is necessary
// and find where to insert individual
int XCSF::individualForDeletion()
{
	//check if deletion is necessary
	if(exceeding_numerosity + population_size < max_population_size)
	{
		//consider that no subsumption is used (increase population)
		//if subsumption is used, it will have to deal with this...
		population_size++;
		return (population_size-1);
	}
		
	//new_individual= rouletteSelection(pop_match_set_size, population_size, random);
	new_individual= random->uniform(0,population_size-1);
	/*if(random->uniform(0.0,1.0)>0.5)
	{
	new_individual= random->uniform(0,population_size-1);
	}
	else
	{
	new_individual= rouletteSelection(pop_match_set_size, population_size, random);
	}
	*/
	
	//select worst individual
	/*new_individual= 0;
	for(i=1;i<population_size;++i)
	{
		if(pop_fitness[i] < pop_fitness[new_individual])
		{
			new_individual= i;
		}
	}
	*/
	

	
	//usual deletion scheme
	/*
	double avg_fitness=0.0;
	//calculate average fitness
	for(i=0;i<population_size;++i)
	{
		avg_fitness+= pop_fitness[i];
	}
	avg_fitness/= (population_size+exceeding_numerosity);

	
	//calculate deletion vote
	for(i=0;i<population_size;++i)
	{
		deletion_vote[i]= 1.0;//pop_match_set_size[i]; //	numerosity[i];
*/
	/*
		
		if(pop_experience[i] > theta_del && pop_fitness[i]/(double)numerosity[i] < delta*avg_fitness)
		{
			deletion_vote[i]= deletion_vote[i]*avg_fitness/(pop_fitness[i]/(double)numerosity[i]);
		}
	}
	
	new_individual= rouletteSelection(deletion_vote, population_size, random);
*/
	//verify the numerosity
	//if it is bigger than 1, than it is just decreased
	if(numerosity[new_individual] > 1)
	{
		numerosity[new_individual]--;
		exceeding_numerosity--;

		population_size++;
		return (population_size-1);
	}
	
	return new_individual;
	
}

//does subsumer subsume the tmp_dna
bool XCSF::doesSubsume(int subsumer, double* tmp_dna)
{
	int action1= getIntegerOutput(tmp_dna);
	int action2= getIntegerOutput(dna[subsumer]);
	
	if(action1 == action2)
	{
		if(canSubsume(subsumer))		
		{
			if(isMoreGeneral(subsumer, tmp_dna))
			{
				return true;
			}
		}
	}

	return false;
}


bool XCSF::canSubsume(int subsumer)
{
	if(pop_experience[subsumer] > theta_sub)
	{
		if(pop_prediction_error[subsumer] < epsilon_zero)
		{
			return true;
		}
	}

	return false;
}

bool XCSF::isMoreGeneral(int subsumer, double* tmp_dna)
{
	int j;

	for(j=0; j<number_of_observation_vars; ++j)
	{
		//dna[2*j]   - minimum of variable j
		//dna[2*j+1] - maximum of variable j
		if(dna[subsumer][2*j] > tmp_dna[2*j])
		{
			return false;
		}
		if(dna[subsumer][2*j+1] < tmp_dna[2*j+1])
		{
			return false;
		}

	}

	return true;
}

void XCSF::deleteOneIndividual()
{
	//store previous population size
	int population_before= population_size;

	int to_delete= individualForDeletion();	

	//check if the population size does not need to delete an individual
	if(population_before < population_size)
	{
		population_size--;
		return;
	}

	if(numerosity[to_delete] > 1)
	{
		numerosity[to_delete]--;
		exceeding_numerosity--;
		return;
	}

	//move the last individual to the individual to be deleted
	copyIndividual(to_delete, population_size-1);
	
	population_size--;

}
		
void XCSF::endEpisode()
{

}

//copy src_index individual to the dst_index individual
void XCSF::copyIndividual(int dst_index, int src_index)
{
	memcpy(dna[dst_index],dna[src_index],sizeof(double)*dna_size);

	//copy variables from the parent
	pop_reward_prediction[dst_index]= pop_reward_prediction[src_index];
	pop_prediction_error[dst_index]= pop_prediction_error[src_index];
	pop_fitness[dst_index]= pop_fitness[src_index];
	memcpy(prediction_weights[dst_index], prediction_weights[src_index],sizeof(double)*(number_of_observation_vars));
	pop_experience[dst_index]= pop_experience[src_index];
	time_stamp[dst_index]= time_stamp[src_index];
	numerosity[dst_index]= numerosity[src_index];

	pop_match_set_size[dst_index]= pop_match_set_size[src_index];
	pop_match_set_size_adjusted_times[dst_index]= pop_match_set_size_adjusted_times[src_index];
	
}

//reset one individual from the population
void XCSF::resetIndividual(int index)
{
	pop_reward_prediction[index]= 10.0;
	pop_fitness[index]= 10.0;
	pop_experience[index]= 0;
	time_stamp[index]= step_counter;
	numerosity[index]= 1;

	pop_match_set_size[index]= 1;
	pop_match_set_size_adjusted_times[index]= 1;

}

double XCSF::stepBestAction(double* observation)
{
	return 0;

}

void XCSF::saveAgent(const char* filename)
{
	printf("saveAgent() not implemented\n");

}

void XCSF::loadAgent(const char* filename)
{
	printf("loadAgent() not implemented\n");
}

