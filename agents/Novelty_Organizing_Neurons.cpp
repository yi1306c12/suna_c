
#include"Novelty_Organizing_Neurons.h"

Novelty_Organizing_Neurons::Novelty_Organizing_Neurons(Random* random)
{
	this->random= random;
	
	//parameters
	gamma= GAMMA_DISCOUNT_FACTOR;
	this->best_solutions_size= BEST_SOLUTIONS_SIZE;
	this->novel_solutions_size= NOVEL_SOLUTIONS_SIZE;
	this->max_population_size= (best_solutions_size + novel_solutions_size)*NOVELTY_POP;
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
	

	this->exploratory= true;

	initPrint();

	episode=0;

	//team initialization
	team_actions=0;
	current_team_fitness=0.0;

	//hall of fame init
	for(int i=0;i<HALL_OF_FAME_SIZE;++i)	
	{
		//negative index, means that no individual is present
		hall_of_fame[i][0]= -1;		
		hall_of_fame_fitness[i]= 0;
	}

#ifdef	PRINT_FITNESS_TABLE	
	fitness_fp= fopen("fitness_table","w");
#endif	
	trial=0;
	evo_fp= fopen("change~hist.txt","w");

}

Novelty_Organizing_Neurons::~Novelty_Organizing_Neurons()
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

void Novelty_Organizing_Neurons::init(int number_of_observation_vars, int number_of_action_vars)
{
	this->number_of_observation_vars= number_of_observation_vars;
	this->number_of_action_vars= number_of_action_vars;

	action= (double*) malloc(number_of_action_vars*sizeof(double));
	tmp_action= (double*) malloc(number_of_action_vars*sizeof(double));
	
	previous_observation_action= (double*) calloc((number_of_action_vars+number_of_observation_vars),sizeof(double));
	
	nmap= new Novelty_Map(NOVELTY_POP, number_of_observation_vars);

	if(use_surrogate)
	{
		//incremental_classifier= new Incremental_Cascade_Correlation(number_of_observation_vars+number_of_action_vars, 1, random);
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
double Novelty_Organizing_Neurons::stepBestAction(double* observation)
{
	int nmap_index;
	
	///////// INPUT TO SOM  /////////

	nmap_index= nmap->inputNeutral(observation);

	//create cell_contents if it does not exist
	if((nmap->map[nmap_index]).pointer==NULL)
	{
		createCellContents(nmap_index);		
	}

	cell_contents* contents= (cell_contents*)(nmap->map[nmap_index]).pointer;
	Individual_Model* model;

	/////////// CHOOSING THE ACTION  ///////
	
	double model_fitness;
	int model_index;

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
			model= population[contents->best_index[i]];		
			model_fitness= contents->best_fitness[i];		

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
		model_index= BEST_SOLUTIONS_SIZE - HALL_OF_FAME_SIZE;
		//model_index= 0;
		model= population[contents->best_index[model_index]];		
		model_fitness= contents->best_fitness[model_index];		

		//set the action to the model's output (no matter which model is used)
		model->getOutput(observation, action);
	}

	return model_fitness;
}

void Novelty_Organizing_Neurons::step(double* observation, double reward)
{
	int nmap_index;

	///////// INPUT TO SOM  /////////

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

	cell_contents* contents= (cell_contents*)(nmap->map[nmap_index]).pointer;
	Individual_Model* model;

	/////////// CHOOSING THE ACTION  ///////
	
	double model_fitness;
	int model_index;
	if(use_surrogate)
	{
		model_index= random->uniform(0, best_solutions_size -1);
		model= population[contents->best_index[model_index]];		
		//model_fitness= contents->best_fitness[model_index];		
		model_fitness= maxFitness(contents);		
	}
	else
	{
		//evaluate best team and individuals
		if(episode < BEST_SOLUTIONS_SIZE)
		{
			//using the last acted individual from this cell
			model_index= episode;
			model= population[contents->best_index[model_index]];
			model_fitness= maxFitness(contents);		
				
			//setting the episode which the model is acting
			contents->when_last_acted= episode;
			contents->index_of_last_acted= contents->best_index[model_index];
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
					model_index= random->uniform(0, novel_solutions_size -1);
					model= population[contents->novel_index[model_index]];		

					//define the model to act this episode
					contents->index_of_last_acted= contents->novel_index[model_index];

					model_fitness= maxFitness(contents);		
				}
				else
				{
					model_index= random->uniform(0, best_solutions_size -1);
					model= population[contents->best_index[model_index]];		
				
					//define the model to act this episode
					contents->index_of_last_acted= contents->best_index[model_index];

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

void Novelty_Organizing_Neurons::endEpisode(double reward)
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
	if(episode>=BEST_SOLUTIONS_SIZE - HALL_OF_FAME_SIZE && episode < BEST_SOLUTIONS_SIZE)
	{
		//convert the index
		int hof_index= episode - BEST_SOLUTIONS_SIZE + HALL_OF_FAME_SIZE;

#ifdef	PRINT_FITNESS_TABLE	
		if(hall_of_fame_fitness[hof_index]!=current_team_fitness)
		{
			fprintf(fitness_fp,"change %d  previous %f test %f\n",trial, hall_of_fame_fitness[hof_index], current_team_fitness);
		}
#endif	
		
		hall_of_fame_fitness[hof_index]= current_team_fitness;
		
	}
	else
	{
		//compare last team's fitness with the fitness required to enter the ranking of best teams
		//insert the team, case it fill the requirements 
		checkHallOfFame(current_team_fitness);
	}
	
	//--------  begin new episode -------
	episode++;
	trial++;

	//restart episode (avoid variable overflow)
	double EVOLUTION_TRIGGER= BEST_SOLUTIONS_SIZE * EXPERIENCE_PER_INDIVIDUAL; 
	if(episode > EVOLUTION_TRIGGER)
	{
		Evolution();
		episode=0;
	}

	//reset team fitness
	team_actions=0;
	current_team_fitness=0.0;
}

void Novelty_Organizing_Neurons::Evolution()
{
	int i,j,k;
	
#ifdef	PRINT_FITNESS_TABLE	
	for(i=0;i<HALL_OF_FAME_SIZE;++i)
	{
		fprintf(fitness_fp,"%d  %f\n",trial, hall_of_fame_fitness[i]);
	}
#endif	
	
	//before evolving, put the best team in the hall of fame to the first hall of fame position
	
	//find best team
	int best_team= 0;
	for(i=BEST_SOLUTIONS_SIZE - HALL_OF_FAME_SIZE;i<BEST_SOLUTIONS_SIZE;++i)
	{
		//convert the index
		int hof_index= i - BEST_SOLUTIONS_SIZE + HALL_OF_FAME_SIZE;

		if(hall_of_fame_fitness[best_team] < hall_of_fame_fitness[hof_index])
		{
			best_team= hof_index;
		}
	}

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
	
	for(k=0;k<NOVELTY_POP;++k)
	{
		cell_contents* c= (cell_contents*)(nmap->map[k]).pointer;
		
		if(c==NULL)
		{
			continue;
		}
	
	
		//Best individuals based on Q-learning fitness (individual based)
		for(i=0;i<BEST_SOLUTIONS_SIZE - HALL_OF_FAME_SIZE;++i)
		{
			double all_best_fitness= c->best_fitness[0];
			bool novel=false;
			int index= 0;

			//find the best individual inside the best solutions
			for(j=i+1;j<BEST_SOLUTIONS_SIZE;++j)
			{
				if(c->best_fitness[j] > all_best_fitness)
				{
					all_best_fitness= c->best_fitness[j];
					novel=false;
					index=j;
				}
			}
		
			//find the best individual inside the novel solutions
			for(j=0;j<NOVEL_SOLUTIONS_SIZE;++j)
			{
				if(c->novel_fitness[j] > all_best_fitness)
				{
					all_best_fitness= c->novel_fitness[j];
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
		
		//create a list with the remaining individuals
		//to keep an index of unused individuals, that will lately be removed from the population
		std::list<int> unused_individuals;
		for(i=BEST_SOLUTIONS_SIZE - HALL_OF_FAME_SIZE;i<BEST_SOLUTIONS_SIZE;++i)
		{
			unused_individuals.push_back(c->best_index[i]);
		}
		for(i=0;i<NOVEL_SOLUTIONS_SIZE;++i)
		{
			unused_individuals.push_back(c->novel_index[i]);
		}

		//Best individuals based on Average fitness (team based)

		for(i=BEST_SOLUTIONS_SIZE - HALL_OF_FAME_SIZE;i<BEST_SOLUTIONS_SIZE;++i)
		{
			//convert the index
			int hof_index= i - BEST_SOLUTIONS_SIZE + HALL_OF_FAME_SIZE;

			//test for empty slot 
			if(hall_of_fame[hof_index][k]==-1)
			{
				printf("Strange, hall of fame = -1 while doing the evolution\n");
				printf("Either evolution is happening too soon or the hall of fame is too big\n");
				exit(1);
			}

			//test for a don't care symbol
			if(hall_of_fame[hof_index][k]==-2)
			{
				//get the last individual of the unused individuals list
				c->best_index[i]= unused_individuals.back();
				c->best_fitness[i]= STARTING_FITNESS;
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
					c->best_index[i]= hall_of_fame[hof_index][k];
					c->best_fitness[i]= STARTING_FITNESS;
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
				c->best_index[i]= hall_of_fame[hof_index][k];
				c->best_fitness[i]= STARTING_FITNESS;
				(population[c->best_index[i]])->increment();
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
				c->novel_index[i]= createOffspring(c,i);
				c->novel_fitness[i]=STARTING_FITNESS;
				c->novel_counter[i]=0.0;
				(population[c->novel_index[i]])->increment();
			}
			else
			{
				c->novel_index[i]= randomIndividual();
				//c->novel_index[i]= c->best_index[i];
				c->novel_fitness[i]=STARTING_FITNESS;
				c->novel_counter[i]=0.0;
				(population[c->novel_index[i]])->increment();
			}
		}

	}
}

void Novelty_Organizing_Neurons::checkHallOfFame(double fitness)
{
	int i;

	//case 1 - hall of fame with empty slots

	//if the last is non existent, there is at least one non existent team in the hall of fame
	if(hall_of_fame[(HALL_OF_FAME_SIZE -1)][0] == -1)
	{
		//find first empty slot in hall of fame
		for(i=0;i<HALL_OF_FAME_SIZE;++i)	
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
	for(i=1;i<HALL_OF_FAME_SIZE;++i)	
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
	

	
}

void Novelty_Organizing_Neurons::copyLastTeamToHallOfFame(int worst_team_index)
{
	int i;

	for(i=0;i<NOVELTY_POP;++i)
	{
		cell_contents* contents= (cell_contents*)(nmap->map[i]).pointer;

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
void Novelty_Organizing_Neurons::localGeneticAlgorithm(int nmap_index, cell_contents* c)
{
	//check if GA should occur
	if(!runOrNotRunGA(c))
	{
		return;	
	}
	
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

//	int a;
//	scanf("%d",&a);

	if(ORIGINAL_DE_OPERATOR)
	{
		//each parent compete with its children
		selectionPressure(c);
		
		//
		removeNovel(c);

		//
		createNovelIndividuals(c);

	}
	else
	{
		//sort best and novel individuals by fitness
		sortByFitness(c);

		//remove novel individuals and update population. Now, with a possible smaller population
		removeNovel(c);

		//use Genetic Operators or Index from the population to create the novel individuals
		//and Clear experience
		createNovelIndividuals(c);
	}


//	clearExperience();

}

void Novelty_Organizing_Neurons::printIndividualFitness(int nmap_index, cell_contents* c)
{
	int i;

	printf("novel individuals' fitness\n");
	for(i=0;i<novel_solutions_size;++i)
	{
		printf("%f\n",c->novel_fitness[i]);
	}
	
	printf("best individuals' fitness\n");
	//double sum=0;
	for(i=0;i<best_solutions_size;++i)
	{
		printf("%f\n",c->best_fitness[i]);
		//sum+= c->best_fitness[i];
	}
	//printf("my %f\n",sum/(double)best_solutions_size);


}

void Novelty_Organizing_Neurons::selectionPressure(cell_contents* c)
{
	int i;

	for(i=0;i<novel_solutions_size;++i)
	{
		//if child has better fitness than its parent,
		//substitute (in fact we exchange the position of the parent and child, so that
		//when we remove the novel individuals, the parent or the child is removed from the population,
		//see function: removeNovel(c) )
		if(c->novel_fitness[i] > c->best_fitness[i])
		{
			int tmp= c->best_index[i];
			c->best_index[i]= c->novel_index[i];
			c->novel_index[i]=tmp;
			double tmp2= c->best_fitness[i];
			c->best_fitness[i]= c->novel_fitness[i];
			c->novel_fitness[i]=tmp2;
		}
	}
}

void Novelty_Organizing_Neurons::sortByFitness()
{
	int i,k;

	//compute average experience of the SOM's neurons
	for(i=0;i<nmap->population_size;++i)
	{
		cell_contents* c= (cell_contents*)(nmap->map[i]).pointer;
			
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
}

void Novelty_Organizing_Neurons::sortByFitness(cell_contents* c)
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

//remove novel individuals and update population. 
void Novelty_Organizing_Neurons::removeNovel()
{
	int i,k;

	//decrement all the novel population
	for(i=0;i<nmap->population_size;++i)
	{
		cell_contents* c= (cell_contents*)(nmap->map[i]).pointer;
		if(c!=NULL)
		{
			for(k=0;k<novel_solutions_size;++k)
			{
				//(population[c->novel_index[k]])->decrement();
				decrement(c->novel_index[k]);
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
void Novelty_Organizing_Neurons::removeNovel(cell_contents* c)
{
	int k;

	//decrement all the novel population
	for(k=0;k<novel_solutions_size;++k)
	{
		decrement(c->novel_index[k]);
		
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

void Novelty_Organizing_Neurons::decrement(int individual)
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
void Novelty_Organizing_Neurons::createNovelIndividuals(cell_contents* c)
{
	int k;

	for(k=0;k<novel_solutions_size;++k)
	{
		if(ORIGINAL_DE_OPERATOR)
		{
			if(random->uniform(0.0,1.0) < PROBABILITY_OF_USING_DE)
			{
				c->novel_index[k]= createOffspring(c,k);
				c->novel_fitness[k]=STARTING_FITNESS;
				c->novel_counter[k]=0.0;
				(population[c->novel_index[k]])->increment();
			}
			else
			{
				c->novel_index[k]= randomIndividual();
				c->novel_fitness[k]=STARTING_FITNESS;
				c->novel_counter[k]=0.0;
				(population[c->novel_index[k]])->increment();

			}

		}
		else
		{
			//create new individual with a genetic operator
			if(random->uniform(0.0,1.0) < PROBABILITY_OF_USING_DE)
			{
				c->novel_index[k]= createOffspring(c);
				c->novel_fitness[k]=STARTING_FITNESS;
				c->novel_counter[k]=0.0;
				(population[c->novel_index[k]])->increment();
			}
			//or index a random individual already existent
			else
			{
				c->novel_index[k]= randomIndividual();
				c->novel_fitness[k]=STARTING_FITNESS;
				c->novel_counter[k]=0.0;
				(population[c->novel_index[k]])->increment();

			}
		}
	}
	
#ifdef	DETERMINISTIC_EXPERIENCE
	for(k=0;k<best_solutions_size;++k)
	{
		c->best_fitness[k]=0.0;
		c->best_counter[k]=0.0;
	}
#endif	

	c->experience=0;
	
}

//create offspring using a genetic operator
int Novelty_Organizing_Neurons::createRandomIndividual()
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
int Novelty_Organizing_Neurons::createOffspring(cell_contents* c, int novel_index)
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
	double* parent_dna= (population[c->best_index[parent]])->dna;

	int r= randomIndividual();
	double* r1_dna= (population[r])->dna;
	r= randomIndividual();
	double* r2_dna= (population[r])->dna;
	r= randomIndividual();
	double* r3_dna= (population[r])->dna;

	int dna_size= (population[r])->dna_size;

#ifdef	RANDOM_F_PARAMETER	
	double F= random->uniform(0.0,2.0);
#else	
	double F= F_PARAMETER;
#endif	
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
int Novelty_Organizing_Neurons::createOffspring(cell_contents* c)
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
	double* parent_dna= (population[c->best_index[parent]])->dna;

	int r= randomIndividual();
	double* r1_dna= (population[r])->dna;
	r= randomIndividual();
	double* r2_dna= (population[r])->dna;
	r= randomIndividual();
	double* r3_dna= (population[r])->dna;

	int dna_size= (population[r])->dna_size;
	
#ifdef	RANDOM_F_PARAMETER	
	double F= random->uniform(0.0,2.0);
#else	
	double F= F_PARAMETER;
#endif	
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
int Novelty_Organizing_Neurons::randomIndividual()
{
	int random_index= random->uniform(0,population_size-1);

	//make sure that the random individual from the population has not been deleted recently (numerosity==0)
	while((population[random_index])->numerosity==0)
	{
		random_index= random->uniform(0,population_size-1);
	}

	return random_index;
}

void Novelty_Organizing_Neurons::clearExperience()
{
	int i;

	for(i=0;i<nmap->population_size;++i)
	{
		cell_contents* c= (cell_contents*)(nmap->map[i]).pointer;
		if(c!=NULL)
		{
			c->experience=0;
		}
	}

}

#ifdef	PRINT_DIVERSITY
void Novelty_Organizing_Neurons::printAverageDiversity()
{
	int i,k,l;
			
	double global_diversity=0.0;
	double global_counter=0;

	for(i=0;i<nmap->population_size;++i)
	{
		cell_contents* c= (cell_contents*)(nmap->map[i]).pointer;
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
#endif

void Novelty_Organizing_Neurons::checkDNALimits(double* dna, int dna_size)
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

void Novelty_Organizing_Neurons::storeAction(int nmap_index, int model_index, double* observation, double* action)
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

void Novelty_Organizing_Neurons::surrogateModelUpdate(double fitness)
{
	double output[1];
	output[0]=fitness;
	
#ifdef PRINT_SURROGATE_ERROR	
	//double* approx_fitness= incremental_classifier->predict(previous_observation_action);

	//fprintf(surrogate_error_fp,"%f\n",sqrt((approx_fitness[0]-fitness)*(approx_fitness[0] - fitness)));
#endif	

	//incremental_classifier->incrementSamples(previous_observation_action, output);
				

}

void Novelty_Organizing_Neurons::reinforcement(double fitness)
{
	int i,j;
	
	if(prev_model == -1)
	{
		return;
	}
	
	//get previous activated cell
	cell_contents* contents= (cell_contents*)nmap->map[prev_nmap_index].pointer;
	
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
				//double* approx_fitness= incremental_classifier->predict(previous_observation_action);

				////////////// approximated fitness evaluate the individual ////////////

				//contents->best_fitness[i]+= widrow_coefficient*(approx_fitness[0] - contents->best_fitness[i]);
				
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
			//double* approx_fitness= incremental_classifier->predict(previous_observation_action);

			////////////// approximated fitness evaluate the individual ////////////

			//contents->novel_fitness[i]+= widrow_coefficient*(approx_fitness[0] - contents->novel_fitness[i]);
			
		}
	}	
	else
	{

		//look for the previous activated model
		for(i=0;i<best_solutions_size;++i)
		{
			if(contents->best_index[i] == contents->index_of_last_acted)
			{
				contents->best_fitness[i]+= widrow_coefficient*(fitness - contents->best_fitness[i]);
				
			}
		}
		
		//look for the previous activated model
		for(i=0;i<novel_solutions_size;++i)
		{
			if(contents->novel_index[i] == contents->index_of_last_acted)
			{
				contents->novel_fitness[i]+= widrow_coefficient*(fitness - contents->novel_fitness[i]);
				
			}
		}

	}
	

#ifdef	PROBABILISTIC_EXPERIENCE
	contents->experience++;
	contents->accum_experience++;
#endif

	//localGeneticAlgorithm(prev_nmap_index, contents);
}

//The first time a cell wins the competition inside the SOM,
//its contents are created by this function
void Novelty_Organizing_Neurons::createCellContents(int nmap_index)
{
	int i;
	cell_contents* contents= (cell_contents*)malloc(sizeof(cell_contents));
	
	//novel individuals are randomly created
	// previously they were chosen from the population
	for(i=0;i<novel_solutions_size;++i)
	{
		//contents->novel_index[i]= random->uniform(0,population_size-1);
		contents->novel_index[i]= createRandomIndividual();
	//	if((population[contents->novel_index[i]])->numerosity < 0)
	//	{
	//		population[contents->novel_index[i]])->numerosity= 
	//	}
		(population[contents->novel_index[i]])->increment();
		contents->novel_fitness[i]=STARTING_FITNESS;
		contents->novel_counter[i]=0;
	}

	for(i=0;i<best_solutions_size;++i)
	{
		//contents->best_index[i]= randomIndividual();
		//contents->best_index[i]= random->uniform(0,population_size-1);
		contents->best_index[i]= createRandomIndividual();
		(population[contents->best_index[i]])->increment();
		contents->best_fitness[i]=STARTING_BEST_FITNESS;
		contents->best_counter[i]=0;
	}
	
	contents->experience= 0;
	contents->accum_experience= 0;

	(nmap->map[nmap_index]).pointer= contents;
}

void Novelty_Organizing_Neurons::print()
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
int Novelty_Organizing_Neurons::chebyshevDistance(int dx, int dy)
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
bool Novelty_Organizing_Neurons::runOrNotRunGA(cell_contents* c)
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

void Novelty_Organizing_Neurons::printExperience()
{
	int i;

	for(i=0;i<nmap->population_size;++i)
	{
		cell_contents* c= (cell_contents*)(nmap->map[i]).pointer;
		
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

void Novelty_Organizing_Neurons::printMap(const char* filename)
{
	int i;

	FILE* fp = fopen(filename,"w");

	for(i=0;i<nmap->population_size;++i)
	{
		cell_contents* c= (cell_contents*)(nmap->map[i]).pointer;
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

void Novelty_Organizing_Neurons::printActionDistribution()
{
	int i,k;

	for(i=0;i<nmap->population_size;++i)
	{
		cell_contents* c= (cell_contents*)(nmap->map[i]).pointer;
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

void Novelty_Organizing_Neurons::printDNA()
{
	int i,k,l;
	
	printf("Print DNA\n");
	for(i=0;i < nmap->population_size;++i)
	{
		cell_contents* contents= (cell_contents*)(nmap->map[i]).pointer;
		if(contents!=NULL)
		{
			for(k=0;k<NOVEL_SOLUTIONS_SIZE;++k)
			{
				Individual_Model* model;

				model= population[contents->novel_index[k]];		

				for(l=0;l<model->dna_size;++l)
				{
					printf("%f ",model->dna[l]);
				}
				
				printf("\n");
			}
			
			for(k=0;k<BEST_SOLUTIONS_SIZE;++k)
			{
				Individual_Model* model;

				model= population[contents->best_index[k]];		

				for(l=0;l<model->dna_size;++l)
				{
					printf("%f ",model->dna[l]);
				}

				printf("\n");
			}
		}
		printf("\n");
	}
	
}

void Novelty_Organizing_Neurons::printCellContents(cell_contents* c)
{
	int k;

	printf("novel\n");
	for(k=0;k<novel_solutions_size;++k)
	{
		printf("index %d fitness %f\n", c->novel_index[k], c->novel_fitness[k]);
	}
	printf("best\n");
	for(k=0;k<best_solutions_size;++k)
	{
		printf("index %d fitness %f\n", c->best_index[k], c->best_fitness[k]);
	}
}

void Novelty_Organizing_Neurons::initPrint()
{
	population_fp= fopen("population_size","w");
}

void Novelty_Organizing_Neurons::finishPrint()
{
	fclose(population_fp);
}

void Novelty_Organizing_Neurons::printPopulationSize()
{
	fprintf(population_fp,"%d\n",population_size - (int)deleted_individuals.size());
}

//return the maximum Fitness of a given cell passed as parameter
double Novelty_Organizing_Neurons::maxFitness(cell_contents* contents)
{
	int k;
	double max_fitness= contents->novel_fitness[0];		
	
	for(k=0;k<novel_solutions_size;++k)
	{
		if(contents->novel_fitness[k] > max_fitness)
		{
			max_fitness= contents->novel_fitness[k];
		}
	}
	
	for(k=0;k<best_solutions_size;++k)
	{
		if(contents->best_fitness[k] > max_fitness)
		{
			max_fitness= contents->best_fitness[k];
		}
	}

	return max_fitness;
}

void Novelty_Organizing_Neurons::saveAgent(const char* filename)
{
	//printf("saveAgent() not implemented\n");

}

void Novelty_Organizing_Neurons::loadAgent(const char* filename)
{
	//printf("loadAgent() not implemented\n");
}

