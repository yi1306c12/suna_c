
#include"NOTC.h"

NOTC::NOTC(Random* random)
{
	this->random= random;
	
	//parameters
	gamma= GAMMA_DISCOUNT_FACTOR;
	this->best_solutions_size= BEST_SOLUTIONS_SIZE;
	this->novel_solutions_size= NOVEL_SOLUTIONS_SIZE;
	this->max_population_size= NUMBER_OF_POPULATIONS*(best_solutions_size + novel_solutions_size)*NOVELTY_POP;
	this->population_size= 0;//max_population_size/4; 
	prev_model=-1;
	widrow_coefficient=WIDROW_COEFFICIENT;
	CR=CR_PARAMETER;
	use_surrogate=USE_SURROGATE;

#ifdef PRINT_SURROGATE_ERROR	
	surrogate_error_fp= fopen("surrogate_error~normal.txt","w");
#endif
#ifdef	PRINT_DIVERSITY		
	diversity_fp= fopen("diversity","w");
#endif		
	if(ORIGINAL_DE_OPERATOR)
	{
		if(BEST_SOLUTIONS_SIZE != NOVEL_SOLUTIONS_SIZE)
		{
			printf("ERROR: Tried using original DE operators, but novel and best solutions size differ\n");
			exit(1);
		}
	}
		
	gen=0;

	this->exploratory= true;

	initPrint();

	episode=0;

	//team initialization
	team_actions=0;
	current_team_fitness=0.0;

	//hall of fame init
	for(int i=0;i<BEST_SOLUTIONS_SIZE;++i)	
	{
		//negative index, means that no individual is present
		hall_of_fame[i][0]= -1;		
		hall_of_fame_fitness[i]= 0;
		hall_of_fame_count[i]= 0;
	}

#ifdef	HIERARCHICAL_HOF
	if(BEST_SOLUTIONS_SIZE%8!=0)
	{
		printf("Cannot create Hierarchical Hall of Fame, best population size is not a multiple of 8\n");
		exit(1);
	}
#endif

#ifdef	PRINT_FITNESS_TABLE	
	fitness_fp= fopen("fitness_table","w");
#endif	
	trial=0;
	evo_fp= fopen("change~hist.txt","w");
}

NOTC::~NOTC()
{
#ifdef PRINT_SURROGATE_ERROR	
	fclose(surrogate_error_fp);
#endif
#ifdef	PRINT_DIVERSITY		
	fclose(diversity_fp);
#endif		
#ifdef	PRINT_FITNESS_TABLE	
	fclose(fitness_fp);
#endif	
	fclose(evo_fp);

	free(action);

	finishPrint();

}

void NOTC::init(int number_of_observation_vars, int number_of_action_vars)
{
	this->number_of_observation_vars= number_of_observation_vars;
	this->number_of_action_vars= number_of_action_vars;

#ifdef RANDOM_INPUT	
	this->number_of_observation_vars++;
#endif	

	action= (double*) malloc(number_of_action_vars*sizeof(double));
	tmp_action= (double*) malloc(number_of_action_vars*sizeof(double));
	
	previous_observation_action= (double*) calloc((number_of_action_vars+number_of_observation_vars),sizeof(double));
	
	nmap= new Novelty_Map(NOVELTY_POP, number_of_observation_vars);

	if(use_surrogate)
	{
		/*
		incremental_classifier= new Incremental_Cascade_Correlation(number_of_observation_vars+number_of_action_vars, 1, random);
		*/
	}
	
	int i;
	//initialize population
	population= new Individual_Model*[max_population_size];
	for(i=0;i<max_population_size;++i)
	{
		switch(INDIVIDUAL_MODEL)
		{
			case NEURAL_MODEL:
			{
				population[i]= new Neural_Model(number_of_observation_vars, NUMBER_OF_HIDDEN_NODES, number_of_action_vars,  NUMBER_OF_HIDDEN_LAYERS, random);

			}
			break;

			case TRIVIAL_MODEL:
			{
				population[i]= new Trivial_Model(number_of_action_vars, random);
			}
			break;
		}
	}
	
}

//give in all steps the best action
//This function is useful to evaluate the algorithm 
double NOTC::stepBestAction(double* observation)
{
	int nmap_index;
	
	///////// INPUT TO SOM  /////////

#ifdef RANDOM_INPUT	
	observation[number_of_observation_vars-1]=random->uniform(-100.0,100.0);
#endif	

	nmap_index= nmap->inputNeutral(observation);

	//create cell_contents if it does not exist
	if((nmap->map[nmap_index]).pointer==NULL)
	{
		createCellContents(nmap_index);		
	}

	complex_cell_contents* contents= (complex_cell_contents*)(nmap->map[nmap_index]).pointer;
	Individual_Model* model;

	/////////// CHOOSING THE ACTION  ///////
	
	double model_fitness;
	int model_index;

	//ENSEMBLE OF ONLY THE FIRST POPULATION INSIDE THE CELL!!!!!
	if(ENSEMBLE_MODE)
	{
		//set action to zero
		for(int i=0;i<number_of_observation_vars;++i)
		{
			action[i]=0.0;
		}

		for(int i=0;i<best_solutions_size;++i)
		{
			//CHOOSE ALWAYS THE BEST ACTION (which means choosing randomly from the best individuals)
			//model_index= random->uniform(0, best_solutions_size -1);
			model= population[contents->best_index[TEAM_POPULATION][i]];		
			model_fitness= contents->best_fitness[TEAM_POPULATION][i];		

			//set the action to the model's output (no matter which model is used)
			model->getOutput(observation, tmp_action);
			
			//set action to zero
			for(int i=0;i<number_of_observation_vars;++i)
			{
				action[i]+= tmp_action[i];
			}

		}
	
		//set the action to the average of the best 
		for(int i=0;i<number_of_observation_vars;++i)
		{
			action[i]/=best_solutions_size;
		}
	}
	else
	{
		//CHOOSE ALWAYS THE BEST ACTION (which means choosing randomly from the best individuals)
		//model_index= random->uniform(0, best_solutions_size -1);
		model_index= 0;
		//model_index= 0;
		model= population[contents->best_index[TEAM_POPULATION][model_index]];		
		model_fitness= contents->best_fitness[TEAM_POPULATION][model_index];		

		//set the action to the model's output (no matter which model is used)
		model->getOutput(observation, action);
	}

	return model_fitness;
}

void NOTC::step(double* observation, double reward)
{
	int nmap_index;

	///////// INPUT TO SOM  /////////
	
#ifdef RANDOM_INPUT	
	observation[number_of_observation_vars-1]=random->uniform(-100.0,100.0);
#endif	

	//find the fittest neuron (closest neuron) in the Novelty Map, related to the observation (input)
	nmap_index= nmap->input(observation);
	//printf("b nmap %d size %d\n",nmap_index,nmap->population_size);

	//create cell_contents if it does not exist
	if((nmap->map[nmap_index]).pointer==NULL)
	{
		createCellContents(nmap_index);		
	}

	//printf("som activated x %d y %d\n",i_index, j_index);
	//som->print();
	//printExperience();

	//int tmp;
	//printf("\n");
	//scanf("%d",&tmp);

	complex_cell_contents* contents= (complex_cell_contents*)(nmap->map[nmap_index]).pointer;
	Individual_Model* model;

	/////////// CHOOSING THE ACTION  ///////
	
	int population_index;
	double model_fitness;
	int model_index;
	if(use_surrogate)
	{
		/*
		model_index= random->uniform(0, best_solutions_size -1);
		model= population[contents->best_index[0][model_index]];		
		//model_fitness= contents->best_fitness[model_index];		
		model_fitness= maxFitness(contents);		
		*/
	}
	else
	{
		
#ifdef	HIERARCHICAL_HOF
		//jump the teams from the teams which are absent from the hall of fame
		if(episode < BEST_SOLUTIONS_SIZE)
		{
			while(hall_of_fame_count[episode]==0)
			{	
				episode++;
			}
		}
#endif

		//evaluate best team and individuals
		if(episode < BEST_SOLUTIONS_SIZE)
		{

			//using the last acted individual from this cell
			model_index= episode;
			model= population[contents->best_index[TEAM_POPULATION][model_index]];
			model_fitness= maxFitness(contents);		
				
			//setting the episode which the model is acting
			contents->when_last_acted= episode;
			contents->index_of_last_acted= contents->best_index[TEAM_POPULATION][model_index];
		}
		else
		{

			//if the individual that is going to be part of the team for this episode
			//was not yet defined, define it
			if(episode != contents->when_last_acted)
			{
				double exploratory_chance= random->uniform(0.0,1.0);
				
				if(exploratory_chance > 0.5)
				{
					population_index= random->uniform(0, NUMBER_OF_POPULATIONS -1);
					model_index= random->uniform(0, novel_solutions_size -1);
					model= population[contents->novel_index[population_index][model_index]];		

					//define the model to act this episode
					contents->index_of_last_acted= contents->novel_index[population_index][model_index];

					model_fitness= maxFitness(contents);		
				}
				else
				{
					population_index= random->uniform(0, NUMBER_OF_POPULATIONS -1);
					model_index= random->uniform(0, best_solutions_size -1);
					model= population[contents->best_index[population_index][model_index]];		
				
					//define the model to act this episode
					contents->index_of_last_acted= contents->best_index[population_index][model_index];

					model_fitness= maxFitness(contents);		
				}

				//setting the episode which the model is acting
				contents->when_last_acted= episode;
			}
			else
			{
				//using the last acted individual from this cell
				model_index= contents->index_of_last_acted;
				model= population[model_index];		
				model_fitness= maxFitness(contents);		
			}
		}
	}

	////////// REINFORCEMENT (FITNESS EVALUATION) //////
	
	//individual reinforcement
	reinforcement(reward + model_fitness*gamma);
	
	//team reinforcement
	current_team_fitness+= reward;

	//set the action to the model's output (changing the model will change this function)
	model->getOutput(observation, action);
	
	//////////  EVOLUTION  ///////
	
	//globalGeneticAlgorithm();

	//store this Action as the Previous Action
	storeAction(nmap_index, model_index, observation, action);

	//exchange between cycles of exploration and cycles of exploitation
	if(EXPLORATION_CYCLES_CHANGE_WITH_SHOCK)
	{
		if(reward < 0.0)
		{
			exploratory= !exploratory;
		}
	}
	else
	{
		exploratory= !exploratory;
	}

	//printDNA();

}

void NOTC::endEpisode(double reward)
{
	//compute the fitness of the last team
	//current_team_fitness= current_team_fitness;
	///(double)team_actions;
	
	//individual reinforcement
	reinforcement(reward);
	
	//team reinforcement
	current_team_fitness+= reward;
	
	//printf("fitness %f\n",current_team_fitness);
	

	//Is the team running one of the hall of fame ones????
	//if it is, update its fitness
	if(episode>=0 && episode < BEST_SOLUTIONS_SIZE)
	{
		//convert the index
		int hof_index= episode;

		//printf("episode %d\n",episode);

#ifdef	PRINT_FITNESS_TABLE	
#ifdef	HIERARCHICAL_HOF
		fprintf(fitness_fp,"trial %d  mean fitness %f current test %f \n",trial, hall_of_fame_fitness[hof_index]/hall_of_fame_count[hof_index], current_team_fitness);
			
#else
		if(hall_of_fame_fitness[hof_index]!=current_team_fitness)
		{
			fprintf(fitness_fp,"change %d  previous %f test %f\n",trial, hall_of_fame_fitness[hof_index], current_team_fitness);
		}

#endif	
#endif	

#ifdef	HIERARCHICAL_HOF
		if(hall_of_fame_count[hof_index]!=0)
		{
			hall_of_fame_fitness[hof_index]+= current_team_fitness;
			hall_of_fame_count[hof_index]++;
		}

		checkHierarchicalAscension(hof_index);
#else
		hall_of_fame_fitness[hof_index]= current_team_fitness;
#endif
	
		
	}
	else
	{
		//compare last team's fitness with the fitness required to enter the ranking of best teams
		//insert the team, case it fill the requirements 
		checkHallOfFame(current_team_fitness);
	}

	//printf("final fitness: %f\n",current_team_fitness);
	
	//--------  begin new episode -------
	episode++;
	trial++;

	//restart episode (avoid variable overflow)
	double EVOLUTION_TRIGGER= BEST_SOLUTIONS_SIZE * EXPERIENCE_PER_INDIVIDUAL/10; 
	if(episode > EVOLUTION_TRIGGER)
	{
		teamEvolution();
		episode=0;
	}

	//reset team fitness
	team_actions=0;
	current_team_fitness=0.0;
}

//check if the hall of the fame's individual can rise in the hierarchy 
bool NOTC::checkHierarchicalAscension(int hof_index)
{
	int i;

	//check current hierarchy
	int current_hierarchy;
	int n= BEST_SOLUTIONS_SIZE;
	if(hof_index >= n -n/4)	
	{
		current_hierarchy=0;
	}
	else
	{
		if(hof_index >= n - n/2)
		{
			current_hierarchy=1;
		}
		else
		{
			if(hof_index >= n - n/2 -n/8)
			{
				current_hierarchy=2;
			}
			else
			{
				if(hof_index >= n - n/2 -n/4)
				{
					current_hierarchy=3;
				}
				else
				{
					if(hof_index >= n - n/2 -n/4 - n/8)
					{
						current_hierarchy=4;
					}
					else
					{
						current_hierarchy=5;

						//cannot ascend more, it is already in the top
						return false;
					}
					
				}
				
			}
		}
	}

	//check the possibility of ascension
	int count= hall_of_fame_count[hof_index];
	int possible_hierarchy;
	if(count < 2)
	{
		possible_hierarchy=0;
	}
	else
	{
		if(count < 4)
		{
			possible_hierarchy=1;
		}
		else
		{
			if(count < 8)
			{
				possible_hierarchy=2;
			}
			else
			{
				if(count < 16)
				{
					possible_hierarchy=3;
				}
				else
				{
					if(count < 32)
					{
						possible_hierarchy=4;
					}
					else
					{
						possible_hierarchy=5;
					}

				}

			}

		}

	}

	//printf("current %d possible %d\n",current_hierarchy, possible_hierarchy);

	//check if the number of times the team was run is enough to rise in the hierarchy
	//if it is not enough, return false
	if(possible_hierarchy <= current_hierarchy)
	{
		return false;
	}

	//try to rise in the hierarchy
	
	//find the limits of the hierarchy
	int next_hierarchy= current_hierarchy + 1;	
	int upper_limit;
	int lower_limit;
	if(next_hierarchy == 1)
	{
		upper_limit=n-n/2;
		lower_limit=n-n/4;
	}
	else
	{
		if(next_hierarchy == 2)
		{
			upper_limit=n-n/2-n/8;
			lower_limit=n-n/2;
		}
		else
		{
			if(next_hierarchy == 3)
			{
				upper_limit= n-n/2-n/4;
				lower_limit= n-n/2-n/8;
			}
			else
			{
				if(next_hierarchy == 4)
				{
					upper_limit=n-n/2-n/4-n/8;
					lower_limit=n-n/2-n/4;
				}
				else
				{
					upper_limit=0;
					lower_limit=n/8;
				}

			}
		}
	}
	
	//search for an team with lower fitness in the upper hierarchy
	int worst_team_index= upper_limit;
	for(i=upper_limit+1;i<lower_limit;++i)	
	{
		//check empty slot
		if(hall_of_fame_count[i]==0)
		{
			worst_team_index= i;
			break;
		}

		if(hall_of_fame_fitness[worst_team_index]/hall_of_fame_count[worst_team_index] > hall_of_fame_fitness[i]/hall_of_fame_count[i])
		{
			worst_team_index= i;
		}
	}

	//insert it
	//if it is better than the worst team from the hall of fame
	//or 
	//if the slot is empty
	if(hall_of_fame_count[worst_team_index]==0 || hall_of_fame_fitness[worst_team_index]/hall_of_fame_count[worst_team_index] < hall_of_fame_fitness[hof_index]/hall_of_fame_count[hof_index])
	{
		swapTeams(hof_index,worst_team_index);

		return true;
	}

	return false;
	

}

void NOTC::swapTeams(int a, int b)
{
	int i;
	//swap teams		
	for(i=0;i<NOVELTY_POP;++i)
	{
		int tmp_index= hall_of_fame[a][i];
		hall_of_fame[a][i]= hall_of_fame[b][i];
		hall_of_fame[b][i]=tmp_index;

	}
	double tmp_fitness= hall_of_fame_fitness[a];
	hall_of_fame_fitness[a]= hall_of_fame_fitness[b];
	hall_of_fame_fitness[b]=tmp_fitness;
	double tmp_count= hall_of_fame_count[a];
	hall_of_fame_count[a]= hall_of_fame_count[b];
	hall_of_fame_count[b]=tmp_count;

}

void NOTC::teamEvolution()
{
	int i,j,k;

	gen++;
	
#ifdef	PRINT_FITNESS_TABLE	
#ifdef	HIERARCHICAL_HOF
	for(i=0;i<BEST_SOLUTIONS_SIZE;++i)
	{
		if(hall_of_fame_count[i]!=0)
		{
			fprintf(fitness_fp,"%d  %f \n",trial, hall_of_fame_fitness[i]/hall_of_fame_count[i]);
		}
		else
		{
			fprintf(fitness_fp,"%d  %f \n",trial, -909090);
		}
	}
#else	
	for(i=0;i<BEST_SOLUTIONS_SIZE;++i)
	{
		fprintf(fitness_fp,"%d  %f\n",trial, hall_of_fame_fitness[i]);
	}
#endif	
#endif	
	//printf("Team Evolution\n");
	//exit(1);
	
	//before evolving, put the best team in the hall of fame in the first hall of fame position
	
#ifndef	HIERARCHICAL_HOF
	//find best team
	int best_team= 0;
	double avg_fitness=0.0;
	for(i=0;i<BEST_SOLUTIONS_SIZE;++i)
	{
		//convert the index
		int hof_index= i;

		//if(hall_of_fame_fitness[best_team]/hall_of_fame_count[best_team] < hall_of_fame_fitness[hof_index]/hall_of_fame_count[hof_index])
		//{
		//	best_team= hof_index;
		//}
		if(hall_of_fame_fitness[best_team] < hall_of_fame_fitness[hof_index])
		{
			best_team= hof_index;
		}

		avg_fitness+= hall_of_fame_fitness[i];
	}
	avg_fitness= avg_fitness/(double)BEST_SOLUTIONS_SIZE;
#ifdef PRINT_AVG_BEST_ACCUM_REWARD	
	printf("%d %f\n", gen, avg_fitness);
#endif	


	//exchange the position 0 of the hall of fame with the best_team position 
	for(i=0;i<NOVELTY_POP;++i)
	{
		int tmp_index= hall_of_fame[0][i];
		hall_of_fame[0][i]= hall_of_fame[best_team][i];
		hall_of_fame[best_team][i]=tmp_index;

	}
	double tmp_fitness= hall_of_fame_fitness[0];
	hall_of_fame_fitness[0]= hall_of_fame_fitness[best_team];
	hall_of_fame_fitness[best_team]=tmp_fitness;
#else
	
	//set the first team to be the best team of the upper hierarchy
	int best_team_index= 0;
	for(i=1;i<BEST_SOLUTIONS_SIZE/8;++i)	
	{
		if(hall_of_fame_fitness[best_team_index]/hall_of_fame_count[best_team_index] > hall_of_fame_fitness[i]/hall_of_fame_count[i])
		{
			best_team_index= i;
		}
	}
	
	swapTeams(best_team_index, 0);
	
	double avg_fitness=0.0;	
	for(i=0;i<BEST_SOLUTIONS_SIZE;++i)
	{
		/*
		if(hall_of_fame_count[i]!=0)
		{
			printf("%d  %f \n",trial, hall_of_fame_fitness[i]/hall_of_fame_count[i]);
		}
		else
		{
			printf("%d  %f \n",trial, -909090);
		}
		*/
		avg_fitness+= hall_of_fame_fitness[i]/hall_of_fame_count[i];
	}
	avg_fitness= avg_fitness/(double)BEST_SOLUTIONS_SIZE;
#ifdef PRINT_AVG_BEST_ACCUM_REWARD	
	printf("%d %f\n", gen, avg_fitness);
#endif	
	


#endif	
	
	for(k=0;k<NOVELTY_POP;++k)
	{
		complex_cell_contents* c= (complex_cell_contents*)(nmap->map[k]).pointer;
		
		if(c==NULL)
		{
			continue;
		}
	
		/*
		//Best individuals based on Q-learning fitness (individual based)
		for(i=0;i<BEST_SOLUTIONS_SIZE;++i)
		{
			double all_best_fitness= c->best_fitness[INDIVIDUALISTIC_POPULATION][0];
			bool novel=false;
			int index= 0;

			//find the best individual inside the best solutions
			for(j=i+1;j<BEST_SOLUTIONS_SIZE;++j)
			{
				if(c->best_fitness[INDIVIDUALISTIC_POPULATION][j] > all_best_fitness)
				{
					all_best_fitness= c->best_fitness[INDIVIDUALISTIC_POPULATION][j];
					novel=false;
					index=j;
				}
			}
		
			//find the best individual inside the novel solutions
			for(j=0;j<NOVEL_SOLUTIONS_SIZE;++j)
			{
				if(c->novel_fitness[INDIVIDUALISTIC_POPULATION][j] > all_best_fitness)
				{
					all_best_fitness= c->novel_fitness[INDIVIDUALISTIC_POPULATION][j];
					novel=true;
					index=j;
				}
				
			}

			//define the new best solution (i.e., just exchange the positions)
			if(novel)
			{
				int tmp= c->best_index[i];
				c->best_index[i]= c->novel_index[index];
				c->novel_index[index]=tmp;
				double tmp2= c->best_fitness[i];
				c->best_fitness[i]= c->novel_fitness[index];
				c->novel_fitness[index]=tmp2;
			}
			else
			{
				int tmp= c->best_index[i];
				c->best_index[i]= c->best_index[index];
				c->best_index[index]=tmp;
				double tmp2= c->best_fitness[i];
				c->best_fitness[i]= c->best_fitness[index];
				c->best_fitness[index]=tmp2;
			}
			
		}
		*/
		
		
		//create a list with the remaining individuals
		//to keep an index of unused individuals, that will lately be removed from the population
		std::list<int> unused_individuals;
		for(i=0;i<BEST_SOLUTIONS_SIZE;++i)
		{
			unused_individuals.push_back(c->best_index[TEAM_POPULATION][i]);
		}
		for(i=0;i<NOVEL_SOLUTIONS_SIZE;++i)
		{
			unused_individuals.push_back(c->novel_index[TEAM_POPULATION][i]);
		}
		

		//Copy Hall of Fame to the Best individuals

		for(i=0;i<BEST_SOLUTIONS_SIZE;++i)
		{
			//convert the index
			int hof_index= i;

			//test for empty slot 
			if(hall_of_fame[hof_index][k]==-1)
			{
#ifndef	HIERARCHICAL_HOF
				printf("Strange, hall of fame = -1 while doing the evolution\n");
				printf("Either evolution is happening too soon or the hall of fame is too big\n");
				exit(1);
#else
				//hierarchical hall of fame will have empty slots in the beginning of the evolution
				c->best_index[TEAM_POPULATION][i]= unused_individuals.back();
				c->best_fitness[TEAM_POPULATION][i]= STARTING_FITNESS;
				unused_individuals.pop_back();
				continue;

#endif
			}

			//test for a don't care symbol
			if(hall_of_fame[hof_index][k]==-2)
			{
				//get the last individual of the unused individuals list
				c->best_index[TEAM_POPULATION][i]= unused_individuals.back();
				c->best_fitness[TEAM_POPULATION][i]= STARTING_FITNESS;
				unused_individuals.pop_back();
				continue;
			}


			//search for a given index inside the unused individuals
			std::list<int>::iterator it;
			bool found=false;
			for(it = unused_individuals.begin(); found == false && it!=unused_individuals.end(); ++it)
			{
				if(hall_of_fame[hof_index][k] == (*it))
				{
					c->best_index[TEAM_POPULATION][i]= hall_of_fame[hof_index][k];
					c->best_fitness[TEAM_POPULATION][i]= STARTING_FITNESS;
					unused_individuals.erase(it);

					found=true;
				}
			}

			//if the hall of fame individual was not found, 
			//then it should be already included in the best_individuals
			if(found==false)
			{
				//printf("found false hof %d k %d\n",hof_index,k);
				//printf("hall of fame %d\n",hall_of_fame[hof_index][k]);
				c->best_index[TEAM_POPULATION][i]= hall_of_fame[hof_index][k];
				c->best_fitness[TEAM_POPULATION][i]= STARTING_FITNESS;
				(population[c->best_index[TEAM_POPULATION][i]])->increment();
			}
		}

		//remove unused individuals
		std::list<int>::iterator it;
		for(it = unused_individuals.begin(); it!=unused_individuals.end(); ++it)
		{
			decrement((*it));
		}

		//reproduction time!
		for(i=0;i<NOVEL_SOLUTIONS_SIZE;++i)
		{
			//printf("reproduction\n");
			if(random->uniform(0.0,1.0) < PROBABILITY_OF_USING_DE)
			{
				c->novel_index[TEAM_POPULATION][i]= createOffspring(c,i,TEAM_POPULATION);
				c->novel_fitness[TEAM_POPULATION][i]=STARTING_FITNESS;
				c->novel_counter[TEAM_POPULATION][i]=0.0;
				(population[c->novel_index[TEAM_POPULATION][i]])->increment();
			}
			else
			{
				c->novel_index[TEAM_POPULATION][i]= randomIndividual();
				//c->novel_index[i]= c->best_index[i];
				c->novel_fitness[TEAM_POPULATION][i]=STARTING_FITNESS;
				c->novel_counter[TEAM_POPULATION][i]=0.0;
				(population[c->novel_index[TEAM_POPULATION][i]])->increment();
			}
		}

	}
}

void NOTC::checkHallOfFame(double fitness)
{
	int i;

#ifndef	HIERARCHICAL_HOF
	//case 1 - hall of fame with empty slots

	//if the last is non existent, there is at least one non existent team in the hall of fame
	if(hall_of_fame[(BEST_SOLUTIONS_SIZE -1)][0] == -1)
	{
		//find first empty slot in hall of fame
		for(i=0;i<BEST_SOLUTIONS_SIZE;++i)	
		{
			if(hall_of_fame[i][0]==-1)
			{
				fprintf(evo_fp,"%d %f\n",trial,fitness);
				//printf("aaaaa %f\n", fitness);
				copyLastTeamToHallOfFame(i);
				
				hall_of_fame_fitness[i]= fitness;
				//printf("aaaaa %f\n", hall_of_fame_fitness[i]);

				//have alredy included the last team on the hall of fame, so return
				return;
			}
		}
	}

	//case 2 - hall of fame with NO empty slots
	
	//search for a position with smaller fitness
	int worst_team_index= 0;
	for(i=1;i<BEST_SOLUTIONS_SIZE;++i)	
	{
		if(hall_of_fame_fitness[worst_team_index] > hall_of_fame_fitness[i])
		{
			worst_team_index= i;
		}
	}
	
	//check if the last team is better than the worst team from the hall of fame
	if( hall_of_fame_fitness[worst_team_index] < fitness)
	{
		fprintf(evo_fp,"%d %f\n",trial, fitness);

		copyLastTeamToHallOfFame(worst_team_index);
			
		hall_of_fame_fitness[worst_team_index]= fitness;
		
		/*
		printf("bbb %f\n", fitness);
	
		for(i=0;i<HALL_OF_FAME_SIZE;++i)
		{
			printf("fitness %f\n",hall_of_fame_fitness[i]);
		}
		*/
	}
#else
	
	//hierarchical hall of fame
	//check the basis of the hierarchy (the lower n/4 individuals of the hall of fame, where n is the size of the hall of fame)
	int worst_team_index= BEST_SOLUTIONS_SIZE -BEST_SOLUTIONS_SIZE/4;
	for(i=BEST_SOLUTIONS_SIZE-BEST_SOLUTIONS_SIZE/4+1;i<BEST_SOLUTIONS_SIZE;++i)	
	{
		//check empty slot
		if(hall_of_fame_count[i]==0)
		{
			worst_team_index= i;
			break;
		}

		if(hall_of_fame_fitness[worst_team_index]/hall_of_fame_count[worst_team_index] > hall_of_fame_fitness[i]/hall_of_fame_count[i])
		{
			worst_team_index= i;
		}
	}

	//insert the last team that acted:
	//if the last team is better than the worst team from the hall of fame
	//or 
	//if the slot is empty
	if(hall_of_fame_count[worst_team_index]==0 || hall_of_fame_fitness[worst_team_index]/hall_of_fame_count[worst_team_index]< fitness)
	{
		fprintf(evo_fp,"%d %f\n",trial, fitness);

		copyLastTeamToHallOfFame(worst_team_index);
			
		hall_of_fame_fitness[worst_team_index]= fitness;
		hall_of_fame_count[worst_team_index]= 1;
	}

	

#endif

	
}

/*
//return if the episode should be finished or not early
bool NOTC::forcedEndOfEpisode()
{
	if()
	{
	}
}
*/

void NOTC::copyLastTeamToHallOfFame(int worst_team_index)
{
	int i;

	for(i=0;i<NOVELTY_POP;++i)
	{
		complex_cell_contents* contents= (complex_cell_contents*)(nmap->map[i]).pointer;

		if(contents==NULL)
		{
			hall_of_fame[worst_team_index][i]=-2;
			continue;
		}

		//verify if the last action was in this episode
		// if yes, insert the last acted
		// if no,  insert a don't care
		if(contents->when_last_acted == episode)
		{
			hall_of_fame[worst_team_index][i]= contents->index_of_last_acted;
		}
		else
		{
			//insert don't care
			hall_of_fame[worst_team_index][i]=-2;
		}

	}
}

//run local genetic algorihtm on the cell passed as parameter
void NOTC::localGeneticAlgorithm(int nmap_index, complex_cell_contents* c)
{
	//check if GA should occur
	if(!runOrNotRunIndividualisticGA(c))
	{
		return;	
	}

	//printf("local GA!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	//return;
	
#ifdef	DETERMINISTIC_EXPERIENCE	
	int k;
	for(k=0;k<best_solutions_size;++k)
	{
		c->best_fitness[k]/= (double)c->best_counter[k];
	}

	for(k=0;k<novel_solutions_size;++k)
	{
		c->novel_fitness[k]/= (double)c->novel_counter[k];
	}
#endif	

	//printExperience();

	//printIndividualFitness(i_index, j_index, c);
//	printIndividualFitness(nmap_index, c);
//	printf("Cell's Experience:  %f\n", c->experience);
	//exit(1);
//	int l;
//	scanf("%d",&l);

//	int a;
//	scanf("%d",&a);
	
	//printf("Individualistic Evolution\n");
	//exit(1);

	if(ORIGINAL_DE_OPERATOR)
	{
		//each parent compete with its children
		selectionPressure(c, INDIVIDUALISTIC_POPULATION);
		
		//
		removeNovel(c, INDIVIDUALISTIC_POPULATION);

		//
		createNovelIndividuals(c, INDIVIDUALISTIC_POPULATION);

	}
	else
	{
		//sort best and novel individuals by fitness
		sortByFitness(c, INDIVIDUALISTIC_POPULATION);

		//remove novel individuals and update population. Now, with a possible smaller population
		removeNovel(c, INDIVIDUALISTIC_POPULATION);

		//use Genetic Operators or Index from the population to create the novel individuals
		//and Clear experience
		createNovelIndividuals(c, INDIVIDUALISTIC_POPULATION);
	}


//	clearExperience();

}

void NOTC::printIndividualFitness(int nmap_index, complex_cell_contents* c)
{
	int i;

	printf("novel individuals' fitness\n");
	for(int p=0; p<NUMBER_OF_POPULATIONS;++p)
	{
		for(i=0;i<novel_solutions_size;++i)
		{
			printf("%f\n",c->novel_fitness[p][i]);
		}
	}
	
	printf("best individuals' fitness\n");
	//double sum=0;
	for(int p=0; p<NUMBER_OF_POPULATIONS;++p)
	{
		for(i=0;i<best_solutions_size;++i)
		{
			printf("%f\n",c->best_fitness[p][i]);
			//sum+= c->best_fitness[i];
		}
	}
	//printf("my %f\n",sum/(double)best_solutions_size);


}

void NOTC::selectionPressure(complex_cell_contents* c, int population_index)
{
	int i;

	for(i=0;i<novel_solutions_size;++i)
	{
		//if child has better fitness than its parent,
		//substitute (in fact we exchange the position of the parent and child, so that
		//when we remove the novel individuals, the parent or the child is removed from the population,
		//see function: removeNovel(c) )
		if(c->novel_fitness[population_index][i] > c->best_fitness[population_index][i])
		{
			int tmp= c->best_index[population_index][i];
			c->best_index[population_index][i]= c->novel_index[population_index][i];
			c->novel_index[population_index][i]=tmp;
			double tmp2= c->best_fitness[population_index][i];
			c->best_fitness[population_index][i]= c->novel_fitness[population_index][i];
			c->novel_fitness[population_index][i]=tmp2;
		}
	}
}

void NOTC::sortByFitness()
{
	/*
	int i,k;

	//compute average experience of the SOM's neurons
	for(i=0;i<nmap->population_size;++i)
	{
		complex_cell_contents* c= (complex_cell_contents*)(nmap->map[i]).pointer;
			
		if(c!=NULL)
		{

			bool inserted=true;
			//In every loop the fittest novel individual try to enter between  
			//the best individuals. When the fittest novel cannot enter, stop the loop.
			while(inserted)
			{
				inserted=false;

				//get best fitness between the novel individuals
				int best_index= 0;
				double best_fitness= c->novel_fitness[0];
				for(k=1;k<novel_solutions_size;++k)
				{
					if(c->novel_fitness[k] > best_fitness)
					{
						best_index=k;
						best_fitness= c->novel_fitness[k];
					}
				}
				
				//try to insert it between the best individuals
				for(k=0;k<best_solutions_size&&inserted==false;++k)
				{
					if(c->best_fitness[k] < best_fitness )
					{
						inserted=true;
						int tmp= c->best_index[k];
						c->best_index[k]= c->novel_index[best_index];
						c->novel_index[best_index]=tmp;
						double tmp2= c->best_fitness[k];
						c->best_fitness[k]= c->novel_fitness[best_index];
						c->novel_fitness[best_index]=tmp2;
					}
				}
				
			}
		}
	}
	*/
}

void NOTC::sortByFitness(complex_cell_contents* c, int population_index)
{
	int k;

	bool inserted=true;
	//In every loop the fittest novel individual try to enter between  
	//the best individuals. When the fittest novel cannot enter, stop the loop.
	while(inserted)
	{
		inserted=false;

		//get best fitness between the novel individuals
		int best_index= 0;
		double best_fitness= c->novel_fitness[population_index][0];
		for(k=1;k<novel_solutions_size;++k)
		{
			if(c->novel_fitness[population_index][k] > best_fitness)
			{
				best_index=k;
				best_fitness= c->novel_fitness[population_index][k];
			}
		}
		
		//try to insert it between the best individuals
		for(k=0;k<best_solutions_size&&inserted==false;++k)
		{
			if(c->best_fitness[population_index][k] < best_fitness )
			{
				inserted=true;
				int tmp= c->best_index[population_index][k];
				c->best_index[population_index][k]= c->novel_index[population_index][best_index];
				c->novel_index[population_index][best_index]=tmp;
				double tmp2= c->best_fitness[population_index][k];
				c->best_fitness[population_index][k]= c->novel_fitness[population_index][best_index];
				c->novel_fitness[population_index][best_index]=tmp2;
			}
		}
		
	}
}

//remove novel individuals and update population. 
void NOTC::removeNovel()
{
	int i,k;

	//decrement all the novel population
	for(i=0;i<nmap->population_size;++i)
	{
		complex_cell_contents* c= (complex_cell_contents*)(nmap->map[i]).pointer;
		if(c!=NULL)
		{
			for(int p=0; p<NUMBER_OF_POPULATIONS;++p)
			{
				for(k=0;k<novel_solutions_size;++k)
				{
					//(population[c->novel_index[k]])->decrement();
					decrement(c->novel_index[p][k]);
				}
			}

		}
	}
/*
	//remove individuals with 0 numerosity (not indexed)
	for(i=population_size -1;i>0;--i)
	{
		Individual_Model* model= population[i];

		//printf("i %d num %d\n",i,model->numerosity);

		if(model->numerosity==0)
		{
			if(i == population_size -1)
			{
				population_size--;
			}
			else
			{
				deleted_individuals.push(i);		
			}
		}
	}
*/
	//printf("pop %d remain %d\n", population_size, population_size-deleted_individuals.size());

	//print individuals marked for deletion
	/*
	for(;!deleted_individuals.empty();)
	{
		printf("deleted %d\n", deleted_individuals.top());
		deleted_individuals.pop();
	}
	*/
	
}

//remove novel individuals and update population. 
void NOTC::removeNovel(complex_cell_contents* c, int population_index)
{
	int k;

	//decrement all the novel population
	for(k=0;k<novel_solutions_size;++k)
	{
		decrement(c->novel_index[population_index][k]);
		
	}

/*	//remove individuals with 0 numerosity (not indexed)
	for(i=population_size -1;i>0;--i)
	{
		Individual_Model* model= population[i];

		//printf("i %d num %d\n",i,model->numerosity);

		if(model->numerosity==0)
		{
			if(i == population_size -1)
			{
				population_size--;
			}
			else
			{
				deleted_individuals.push(i);		
			}
		}
	}
*/	
	
}

void NOTC::decrement(int individual)
{
	(population[individual])->decrement();
	
	Individual_Model* model= population[individual];

	//printf("i %d num %d\n",i,model->numerosity);

	if(model->numerosity==0)
	{
		if(individual == population_size -1)
		{
			population_size--;
		}
		else
		{
			deleted_individuals.push(individual);		
		}
	}
}


//use Genetic Operators or Index from the population to create the novel individuals
//and clear experience
void NOTC::createNovelIndividuals(complex_cell_contents* c, int population_index)
{
	int k;

	for(k=0;k<novel_solutions_size;++k)
	{
		if(ORIGINAL_DE_OPERATOR)
		{
			if(random->uniform(0.0,1.0) < PROBABILITY_OF_USING_DE)
			{
				c->novel_index[population_index][k]= createOffspring(c,k, population_index);
				c->novel_fitness[population_index][k]=STARTING_FITNESS;
				c->novel_counter[population_index][k]=0.0;
				(population[c->novel_index[population_index][k]])->increment();
			}
			else
			{
				c->novel_index[population_index][k]= randomIndividual();
				c->novel_fitness[population_index][k]=STARTING_FITNESS;
				c->novel_counter[population_index][k]=0.0;
				(population[c->novel_index[population_index][k]])->increment();

			}

		}
		else
		{
			//create new individual with a genetic operator
			if(random->uniform(0.0,1.0) < PROBABILITY_OF_USING_DE)
			{
				c->novel_index[population_index][k]= createOffspring(c, population_index);
				c->novel_fitness[population_index][k]=STARTING_FITNESS;
				c->novel_counter[population_index][k]=0.0;
				(population[c->novel_index[population_index][k]])->increment();
			}
			//or index a random individual already existent
			else
			{
				c->novel_index[population_index][k]= randomIndividual();
				c->novel_fitness[population_index][k]=STARTING_FITNESS;
				c->novel_counter[population_index][k]=0.0;
				(population[c->novel_index[population_index][k]])->increment();

			}
		}
	}
	
#ifdef	DETERMINISTIC_EXPERIENCE
	for(k=0;k<best_solutions_size;++k)
	{
		c->best_fitness[population_index][k]=0.0;
		c->best_counter[population_index][k]=0.0;
	}
#endif	

	c->experience=0;
	
}

//create offspring using a genetic operator
int NOTC::createRandomIndividual()
{
	int i;

	//get index
	int offspring_index;
	//if the stack is not empty, get the top of the stack
	if(!deleted_individuals.empty())
	{
		offspring_index= deleted_individuals.top();
		deleted_individuals.pop();
	}
	//if the stack is empty, increase population
	else
	{
		offspring_index= population_size;
		population_size++;
	}
	
	///////////////// Set the DNA to a Random DNA ///////////////

	double* offspring_dna= (population[offspring_index])->dna;

	for(i=0;i<(population[offspring_index])->dna_size;++i)
	{
		offspring_dna[i]= random->uniform(-1.0,1.0);
	}

	(population[offspring_index])->updateModel(offspring_dna);

	return offspring_index;
}

//create offspring using a genetic operator
int NOTC::createOffspring(complex_cell_contents* c, int novel_index, int population_index)
{
	//get index
	int offspring_index;
	//if the stack is not empty, get the top of the stack
	if(!deleted_individuals.empty())
	{
		offspring_index= deleted_individuals.top();
		deleted_individuals.pop();
	}
	//if the stack is empty, increase population
	else
	{
		offspring_index= population_size;
		population_size++;
	}
	
	///////////////// Apply a Genetic Operator (Similar to Differential Evolution) ///////////////

	double* offspring_dna= (population[offspring_index])->dna;

	int parent= novel_index;
	double* parent_dna= (population[c->best_index[population_index][parent]])->dna;

	int r= randomIndividual();
	double* r1_dna= (population[r])->dna;
	r= randomIndividual();
	double* r2_dna= (population[r])->dna;
	r= randomIndividual();
	double* r3_dna= (population[r])->dna;

	int dna_size= (population[r])->dna_size;
	
	double F= F_PARAMETER;
	//double F= random->uniform(0.0,1.0);
	//double F= 2.0;
	
	differentialEvolutionOperator(offspring_dna, CR, F, parent_dna, r1_dna, r2_dna, r3_dna, dna_size, random);

	if(DNA_LIMITS)
	{
		checkDNALimits(offspring_dna, population[0]->dna_size);
	}

	//make sure the Model is updated with the new DNA
	(population[offspring_index])->updateModel(offspring_dna);

	return offspring_index;
}

//create offspring using a genetic operator
int NOTC::createOffspring(complex_cell_contents* c, int population_index)
{
	//get index
	int offspring_index;
	//if the stack is not empty, get the top of the stack
	if(!deleted_individuals.empty())
	{
		offspring_index= deleted_individuals.top();
		deleted_individuals.pop();
	}
	//if the stack is empty, increase population
	else
	{
		offspring_index= population_size;
		population_size++;
	}
	
	///////////////// Apply a Genetic Operator (Similar to Differential Evolution) ///////////////

	double* offspring_dna= (population[offspring_index])->dna;

	int parent= random->uniform(0, best_solutions_size-1);
	double* parent_dna= (population[c->best_index[population_index][parent]])->dna;

	int r= randomIndividual();
	double* r1_dna= (population[r])->dna;
	r= randomIndividual();
	double* r2_dna= (population[r])->dna;
	r= randomIndividual();
	double* r3_dna= (population[r])->dna;

	int dna_size= (population[r])->dna_size;
	
	double F= F_PARAMETER;
	//double F= random->uniform(0.0,1.0);
	//double F= 2.0;
	
	differentialEvolutionOperator(offspring_dna, CR, F, parent_dna, r1_dna, r2_dna, r3_dna, dna_size, random);

	if(DNA_LIMITS)
	{
		checkDNALimits(offspring_dna, population[0]->dna_size);
	}

	//make sure the Model is updated with the new DNA
	(population[offspring_index])->updateModel(offspring_dna);

	return offspring_index;
}

//Index a random individual already existent
//i.e., try an already existent individual from a probably different cell
int NOTC::randomIndividual()
{
	int random_index= random->uniform(0,population_size-1);

	//make sure that the random individual from the population has not been deleted recently (numerosity==0)
	while((population[random_index])->numerosity==0)
	{
		random_index= random->uniform(0,population_size-1);
	}

	return random_index;
}

void NOTC::clearExperience()
{
	int i;

	for(i=0;i<nmap->population_size;++i)
	{
		complex_cell_contents* c= (complex_cell_contents*)(nmap->map[i]).pointer;
		if(c!=NULL)
		{
			c->experience=0;
		}
	}

}

#ifdef	PRINT_DIVERSITY
/*
void NOTC::printAverageDiversity()
{
	int i,k,l;
			
	double global_diversity=0.0;
	double global_counter=0;

	for(i=0;i<nmap->population_size;++i)
	{
		complex_cell_contents* c= (complex_cell_contents*)(nmap->map[i]).pointer;
		double avg_diversity=0.0;

		if(contents!=NULL)
		{
		
			double counter=0;
			Individual_Model* model;
			Individual_Model* model_reference;
			for(k=0;k<novel_solutions_size;++k)
			{
				model= population[contents->novel_index[k]];

				for(l=k;l<novel_solutions_size;++l)
				{
					model_reference= population[contents->novel_index[l]];
					
					avg_diversity+= model->diversity(model_reference);
					counter++;
				}

				for(l=0;l<best_solutions_size;++l)
				{
					model_reference= population[contents->best_index[l]];
					
					avg_diversity+= model->diversity(model_reference);
					counter++;
				}

			}
			
			for(k=0;k<best_solutions_size;++k)
			{
				model= population[contents->best_index[k]];

				for(l=k;l<best_solutions_size;++l)
				{
					model_reference= population[contents->best_index[l]];
					
					avg_diversity+= model->diversity(model_reference);
					counter++;
				}

			
				avg_diversity= avg_diversity/counter;
			}

			global_diversity+= avg_diversity;
			global_counter++;

		}
		
	}

	if(global_counter!=0)
	{
		global_diversity= global_diversity/global_counter;
				
		fprintf(diversity_fp,"%f\n",global_diversity);
	}

}
*/
#endif

void NOTC::checkDNALimits(double* dna, int dna_size)
{
	int i;
	for(i=0;i<dna_size;++i)
	{
		if(dna[i]>1.0)
		{
			dna[i]=1.0;
		}
		else
		{
			if(dna[i]<-1.0)
			{
				dna[i]=-1.0;
			}
		}
	}
}

void NOTC::storeAction(int nmap_index, int model_index, double* observation, double* action)
{
	int i;

#ifdef	PRINT_DIVERSITY
	printAverageDiversity();
#endif	

	prev_nmap_index= nmap_index;
	prev_exploratory= exploratory;
	prev_model= model_index;

	for(i=0;i<number_of_observation_vars;++i)
	{
		previous_observation_action[i]= observation[i];
	}
	
	for(;i<number_of_action_vars+number_of_observation_vars;++i)
	{
		previous_observation_action[i]= action[i-number_of_observation_vars];
	}
}

void NOTC::surrogateModelUpdate(double fitness)
{
	double output[1];
	output[0]=fitness;
	
#ifdef PRINT_SURROGATE_ERROR	
	//double* approx_fitness= incremental_classifier->predict(previous_observation_action);

	//fprintf(surrogate_error_fp,"%f\n",sqrt((approx_fitness[0]-fitness)*(approx_fitness[0] - fitness)));
#endif	

	//incremental_classifier->incrementSamples(previous_observation_action, output);
				

}

void NOTC::reinforcement(double fitness)
{
	int i,j;
	
	if(prev_model == -1)
	{
		return;
	}
	
	//get previous activated cell
	complex_cell_contents* contents= (complex_cell_contents*)nmap->map[prev_nmap_index].pointer;
	
	/*
	if(use_surrogate)
	{
		
		surrogateModelUpdate(fitness);
		
		//only best individuals are chosen for action and therefore only them receive 
		//the reward from the environment, the remaining individuals use the surrogate model
		contents->best_fitness[prev_model]+= widrow_coefficient*(fitness - contents->best_fitness[prev_model]);

		//update the fitness of the other individuals using the surrogate model
		for(i=0;i<BEST_SOLUTIONS_SIZE;++i)
		{
			//exclude the individual that acted previously
			if(i!=prev_model)
			{
				Individual_Model* model;

				model= population[contents->best_index[i]];		

				////////////// get the approximated fitness /////////

				//get the output of the individual for the previous action
				model->getOutput(previous_observation_action, tmp_action);
			
				//substitute the previous action for this individual's action
				for(j=number_of_observation_vars;j<number_of_action_vars+number_of_observation_vars;++j)
				{
					previous_observation_action[j]= tmp_action[j-number_of_observation_vars];
				}

				//get the predicted fitness
				double* approx_fitness= incremental_classifier->predict(previous_observation_action);

				////////////// approximated fitness evaluate the individual ////////////

				contents->best_fitness[i]+= widrow_coefficient*(approx_fitness[0] - contents->best_fitness[i]);
				
			}
		}
		
		
		for(i=0;i<NOVEL_SOLUTIONS_SIZE;++i)
		{
			Individual_Model* model;

			model= population[contents->novel_index[i]];		

			////////////// get the approximated fitness /////////

			//get the output of the individual for the previous action
			model->getOutput(previous_observation_action, tmp_action);
		
			//substitute the previous action for this individual's action
			for(j=number_of_observation_vars;j<number_of_action_vars+number_of_observation_vars;++j)
			{
				previous_observation_action[j]= tmp_action[j-number_of_observation_vars];
			}

			//get the predicted fitness
			double* approx_fitness= incremental_classifier->predict(previous_observation_action);

			////////////// approximated fitness evaluate the individual ////////////

			contents->novel_fitness[i]+= widrow_coefficient*(approx_fitness[0] - contents->novel_fitness[i]);
			
		}
		
	}	
	else	*/
		
		//update the fitness of all individuals which have the same DNA as the individual
		//which acted  last iteration
		bool used_individual_from_the_individualist_population=false;

		//look for the previous activated model
		//for(int p=0; p<NUMBER_OF_POPULATIONS;++p)
		int p= INDIVIDUALISTIC_POPULATION;
		
		for(i=0;i<best_solutions_size;++i)
		{
			if(contents->best_index[p][i] == contents->index_of_last_acted)
			{
				contents->best_fitness[p][i]+= widrow_coefficient*(fitness - contents->best_fitness[p][i]);
				
			}
	
			if(p==INDIVIDUALISTIC_POPULATION)
			{
				used_individual_from_the_individualist_population=true;
			}
		}
		
		//look for the previous activated model
		//for(int p=0; p<NUMBER_OF_POPULATIONS;++p)
		//{
		
		p= INDIVIDUALISTIC_POPULATION;
		for(i=0;i<novel_solutions_size;++i)
		{
			if(contents->novel_index[p][i] == contents->index_of_last_acted)
			{
				contents->novel_fitness[p][i]+= widrow_coefficient*(fitness - contents->novel_fitness[p][i]);
				
			}
			
			if(p==INDIVIDUALISTIC_POPULATION)
			{
				used_individual_from_the_individualist_population=true;
			}
		}
		//}

	

#ifdef	PROBABILISTIC_EXPERIENCE
	if(used_individual_from_the_individualist_population==true)
	{
		contents->experience++;
		contents->accum_experience++;
	}
#endif

	localGeneticAlgorithm(prev_nmap_index, contents);
}

//The first time a cell wins the competition inside the SOM,
//its contents are created by this function
void NOTC::createCellContents(int nmap_index)
{
	int i;
	complex_cell_contents* contents= (complex_cell_contents*)malloc(sizeof(complex_cell_contents));
	
	//novel individuals are randomly created
	// previously they were chosen from the population
	for(int p=0; p<NUMBER_OF_POPULATIONS;++p)
	{
		for(i=0;i<novel_solutions_size;++i)
		{
			//contents->novel_index[i]= random->uniform(0,population_size-1);
			contents->novel_index[p][i]= createRandomIndividual();
		//	if((population[contents->novel_index[i]])->numerosity < 0)
		//	{
		//		population[contents->novel_index[i]])->numerosity= 
		//	}
			(population[contents->novel_index[p][i]])->increment();
			contents->novel_fitness[p][i]=STARTING_FITNESS;
			contents->novel_counter[p][i]=0;
		}
	}

	for(int p=0; p<NUMBER_OF_POPULATIONS;++p)
	{
		for(i=0;i<best_solutions_size;++i)
		{
			//contents->best_index[i]= randomIndividual();
			//contents->best_index[i]= random->uniform(0,population_size-1);
			contents->best_index[p][i]= createRandomIndividual();
			(population[contents->best_index[p][i]])->increment();
			contents->best_fitness[p][i]=STARTING_BEST_FITNESS;
			contents->best_counter[p][i]=0;
		}
	}
	
	contents->experience= 0;
	contents->accum_experience= 0;

	(nmap->map[nmap_index]).pointer= contents;
}

void NOTC::print()
{
	printPopulationSize();
/*	
	printf("SOM learning rate %f neighborhood parameter %f\n",som->learning_rate, som->neighborhood_parameter);
	som->print();
	printExperience();
	printActionDistribution();

	printf("Extremes\n");
	double input[2];
	input[0]=10;
	input[1]=10;
	int i_index, j_index;
	som->neuronCompetition(input, i_index, j_index);
	printf("\nmax max %d %d\n",i_index, j_index);
	som->map[i_index][j_index]->print();
	input[0]=10;
	input[1]=0;
	som->neuronCompetition(input, i_index, j_index);
	printf("\nmax min %d %d\n",i_index, j_index);
	som->map[i_index][j_index]->print();
	input[0]=0;
	input[1]=10;
	som->neuronCompetition(input, i_index, j_index);
	printf("\nmin max %d %d\n",i_index, j_index);
	som->map[i_index][j_index]->print();
	input[0]=0;
	input[1]=0;
	som->neuronCompetition(input, i_index, j_index);
	printf("\nmin min %d %d\n",i_index, j_index);
	som->map[i_index][j_index]->print();
*/

}

/***********************
 * Chebyshev distances:
 *	|2|1|1|1|2|
 *	|2|1|0|1|2|
 *	|2|1|1|1|2|
 * ********************/
int NOTC::chebyshevDistance(int dx, int dy)
{
	if(dx > dy)
	{
		return dx;
	}
	else
	{
		return dy;
	}
}


//check if GA should or not run - return true if it should run
//
//Run GA if on average, the cell_contents have more than EXPERIENCE_PER_INDIVIDUAL*novel_solutions_size
bool NOTC::runOrNotRunIndividualisticGA(complex_cell_contents* c)
{
	double avg_experience= c->experience;

#ifdef	PROBABILISTIC_EXPERIENCE
	if(avg_experience > EXPERIENCE_PER_INDIVIDUAL*(novel_solutions_size+best_solutions_size))
	{
		//printf("avg exp %f\n",avg_experience);
		return true;
	}
#endif	
#ifdef	DETERMINISTIC_EXPERIENCE	
	if(avg_experience > (novel_solutions_size+best_solutions_size))
	{
		//printf("avg exp %f\n",avg_experience);
		return true;
	}
#endif	
	
	return false;
}

void NOTC::printExperience()
{
	int i;

	for(i=0;i<nmap->population_size;++i)
	{
		complex_cell_contents* c= (complex_cell_contents*)(nmap->map[i]).pointer;
		
		if(c==NULL)
		{
			printf("%3.0f, ",-1.0);
		}
		else
		{
			printf("%3.0lf, ",c->experience);
		}
		printf("\n");
	}

}

void NOTC::printMap(const char* filename)
{
	int i;

	FILE* fp = fopen(filename,"w");

	for(i=0;i<nmap->population_size;++i)
	{
		complex_cell_contents* c= (complex_cell_contents*)(nmap->map[i]).pointer;
		if(c!=NULL)
		{
			fprintf(fp,"%3f \n",c->experience);
		}
		else
		{
			fprintf(fp,"0 \n");
		}
	}
	fclose(fp);
}

/*
void NOTC::printActionDistribution()
{
	int i,k;

	for(i=0;i<nmap->population_size;++i)
	{
		complex_cell_contents* c= (complex_cell_contents*)(nmap->map[i]).pointer;
		if(c==NULL)
		{
			printf("%3.0f, ",0.0);
		}
		else
		{
			int best_index= 0;
			double best_fitness= c->best_fitness[0];
			for(k=1;k<best_solutions_size;++k)
			{
				if(c->best_fitness[k] > best_fitness)
				{
					best_index=k;
					best_fitness= c->best_fitness[k];
				}
			}
			(population[c->best_index[best_index]])->getOutput(NULL,action);
			printf("(");
			for(k=0;k<number_of_action_vars;++k)
			{
				printf("%3.1lf, ",action[k]);
			}
			printf(") ");
		}
		printf("\n");
	}

}
*/

void NOTC::printDNA()
{
	int i,k,l;
	
	printf("Print DNA\n");
	for(i=0;i < nmap->population_size;++i)
	{
		complex_cell_contents* contents= (complex_cell_contents*)(nmap->map[i]).pointer;
		if(contents!=NULL)
		{
			for(int p=0; p<NUMBER_OF_POPULATIONS;++p)
			{
				for(k=0;k<NOVEL_SOLUTIONS_SIZE;++k)
				{
					Individual_Model* model;

					model= population[contents->novel_index[p][k]];		

					for(l=0;l<model->dna_size;++l)
					{
						printf("%f ",model->dna[l]);
					}
					
					printf("\n");
				}
				
				for(k=0;k<BEST_SOLUTIONS_SIZE;++k)
				{
					Individual_Model* model;

					model= population[contents->best_index[p][k]];		

					for(l=0;l<model->dna_size;++l)
					{
						printf("%f ",model->dna[l]);
					}

					printf("\n");
				}
			}
		}
		printf("\n");
	}
	
}

void NOTC::printCellContents(complex_cell_contents* c)
{
	int k;

	printf("novel\n");
	for(int p=0; p<NUMBER_OF_POPULATIONS;++p)
	{
		for(k=0;k<novel_solutions_size;++k)
		{
			printf("index %d fitness %f\n", c->novel_index[p][k], c->novel_fitness[p][k]);
		}
	}
	printf("best\n");
	for(int p=0; p<NUMBER_OF_POPULATIONS;++p)
	{
		for(k=0;k<best_solutions_size;++k)
		{
			printf("index %d fitness %f\n", c->best_index[p][k], c->best_fitness[p][k]);
		}
	}
}

void NOTC::initPrint()
{
	population_fp= fopen("population_size","w");
}

void NOTC::finishPrint()
{
	fclose(population_fp);
}

void NOTC::printPopulationSize()
{
	fprintf(population_fp,"%d\n",population_size - (int)deleted_individuals.size());
}

//return the maximum Fitness of a given cell passed as parameter
double NOTC::maxFitness(complex_cell_contents* contents)
{
	int k;
	int p=INDIVIDUALISTIC_POPULATION;
	double max_fitness= contents->novel_fitness[p][0];		
	
	for(k=0;k<novel_solutions_size;++k)
	{
		if(contents->novel_fitness[p][k] > max_fitness)
		{
			max_fitness= contents->novel_fitness[p][k];
		}
	}
	
	for(k=0;k<best_solutions_size;++k)
	{
		if(contents->best_fitness[p][k] > max_fitness)
		{
			max_fitness= contents->best_fitness[p][k];
		}
	}

	return max_fitness;
}

// save the complete agent
// 	-dna of each subpop in each cell
// 	-hall of fame
// 	-nmap positions
void NOTC::saveAgent(const char* filename)
{
	int i,k,l;

	/*
	FILE* fp= fopen(filename,"w");
			
	for(i=0;i < nmap->population_size;++i)
	{
		complex_cell_contents* contents= (complex_cell_contents*)(nmap->map[i]).pointer;
		if(contents!=NULL)
		{
			fprinf(fp,"nonempty\n");

			//print Novelty Map weight array
			for(j=0;j<(nmap->map[i])->size;++j)
			{
				fprintf(fp,"%f ",(nmap->map[i])->weight[j]);
			}
			fprintf(fp,"\n");

			//saving the dna inside each cell
			for(int p=0; p<NUMBER_OF_POPULATIONS;++p)
			{
				for(k=0;k<NOVEL_SOLUTIONS_SIZE;++k)
				{
					Individual_Model* model;

					fprintf(fp,"%f ",contents->novel_index[p]);

					model= population[contents->novel_index[p][k]];		

					for(l=0;l<model->dna_size;++l)
					{
						fprintf(fp,"%f ",model->dna[l]);
					}
					
					fprintf(fp,"\n");
				}
				
				for(k=0;k<BEST_SOLUTIONS_SIZE;++k)
				{
					Individual_Model* model;

					model= population[contents->best_index[p][k]];		

					for(l=0;l<model->dna_size;++l)
					{
						fprintf(fp,"%f ",model->dna[l]);
					}

					fprintf(fp,"\n");
				}
			}
		}
		else
		{
			fprinf(fp,"empty\n");
		}
	}
	
	//print hall of fame
	for(i=0;i<BEST_SOLUTIONS_SIZE;++i)
	{
		fprintf(hall_of_fame[BEST_SOLUTIONS_SIZE][NOVELTY_POP];	//NOVELTY_POP is the number of individuals in a team
		double hall_of_fame_fitness[BEST_SOLUTIONS_SIZE];
	}
	

	fclose(fp);
	*/
}

void NOTC::loadAgent(const char* filename)
{
	printf("loadAgent() not implemented\n");
}

