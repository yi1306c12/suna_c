
#include"Self_Organizing_Neurons.h"

Self_Organizing_Neurons::Self_Organizing_Neurons(int som_width, int som_height, Random* random)
{
	this->random= random;
	
	//parameters
	gamma= GAMMA_DISCOUNT_FACTOR;
	this->som_width= som_width;
	this->som_height= som_height;
	this->best_solutions_size= BEST_SOLUTIONS_SIZE;
	this->novel_solutions_size= NOVEL_SOLUTIONS_SIZE;
	this->max_population_size= (best_solutions_size + novel_solutions_size)*som_width*som_height;
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
	surrogate_counter=0;

	initPrint();
}

Self_Organizing_Neurons::~Self_Organizing_Neurons()
{
#ifdef PRINT_SURROGATE_ERROR	
	fclose(surrogate_error_fp);
#endif
#ifdef	PRINT_DIVERSITY		
	fclose(diversity_fp);
#endif		

	free(action);

	finishPrint();

}

void Self_Organizing_Neurons::init(int number_of_observation_vars, int number_of_action_vars)
{
	this->number_of_observation_vars= number_of_observation_vars;
	this->number_of_action_vars= number_of_action_vars;

	action= (double*) malloc(number_of_action_vars*sizeof(double));
	tmp_action= (double*) malloc(number_of_action_vars*sizeof(double));
	
	previous_observation_action= (double*) calloc((number_of_action_vars+number_of_observation_vars),sizeof(double));
	
	som= new Som_Map(som_width, som_height, number_of_observation_vars);

	//som->print();
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
double Self_Organizing_Neurons::stepBestAction(double* observation)
{
	int i_index;
	int j_index;
	
	///////// INPUT TO SOM  /////////

	//find the fittest neuron (closest neuron) in the SOM, related to the observation (input)
	som->neuronCompetition(observation, i_index, j_index);

	//update the neighborhood with the observation (SOM's usual behavior)
	//som->updateNeighborhood(observation, i_index, j_index);	

	//create cell_contents if it does not exist
	if((som->map[i_index][j_index])->pointer==NULL)
	{
		createCellContents(i_index, j_index);		
	}

	cell_contents* contents= (cell_contents*)(som->map[i_index][j_index])->pointer;
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
		model_index= random->uniform(0, best_solutions_size -1);
		model= population[contents->best_index[model_index]];		
		model_fitness= contents->best_fitness[model_index];		

		//set the action to the model's output (no matter which model is used)
		model->getOutput(observation, action);
	}

	return model_fitness;
}

void Self_Organizing_Neurons::step(double* observation, double reward)
{
	int i_index;
	int j_index;

	///////// INPUT TO SOM  /////////

	//find the fittest neuron (closest neuron) in the SOM, related to the observation (input)
	som->neuronCompetition(observation, i_index, j_index);

	//update the neighborhood with the observation (SOM's usual behavior)
	som->updateNeighborhood(observation, i_index, j_index);	

	//create cell_contents if it does not exist
	if((som->map[i_index][j_index])->pointer==NULL)
	{
		createCellContents(i_index, j_index);		
	}

	//printf("som activated x %d y %d\n",i_index, j_index);
	//som->print();
	//printExperience();

	//int tmp;
	//printf("\n");
	//scanf("%d",&tmp);

	cell_contents* contents= (cell_contents*)(som->map[i_index][j_index])->pointer;
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
		if(exploratory)
		{
			model_index= random->uniform(0, novel_solutions_size -1);
			model= population[contents->novel_index[model_index]];		
			//model_fitness= contents->novel_fitness[model_index];		
			model_fitness= maxFitness(contents);		
		}
		else
		{
			model_index= random->uniform(0, best_solutions_size -1);
			model= population[contents->best_index[model_index]];		
			//model_fitness= contents->best_fitness[model_index];		
			model_fitness= maxFitness(contents);		
		}
	}

	////////// REINFORCEMENT (FITNESS EVALUATION) //////
	
	reinforcement(reward + model_fitness*gamma);

	//set the action to the model's output (changing the model will change this function)
	model->getOutput(observation, action);
	
	//////////  EVOLUTION  ///////
	
	//globalGeneticAlgorithm();

	//store this Action as the Previous Action
	storeAction(i_index, j_index, model_index, observation, action);

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

//run local genetic algorihtm on the cell passed as parameter
void Self_Organizing_Neurons::localGeneticAlgorithm(int i_index, int j_index, cell_contents* c)
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
		createNovelIndividuals(i_index, j_index, c);

	}
	else
	{
		//sort best and novel individuals by fitness
		sortByFitness(c);

		//remove novel individuals and update population. Now, with a possible smaller population
		removeNovel(c);

		//use Genetic Operators or Index from the population to create the novel individuals
		//and Clear experience
		createNovelIndividuals(i_index, j_index, c);
	}


//	clearExperience();

}

void Self_Organizing_Neurons::printIndividualFitness(int i_index, int j_index, cell_contents* c)
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

void Self_Organizing_Neurons::selectionPressure(cell_contents* c)
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

void Self_Organizing_Neurons::sortByFitness()
{
	int i,j,k;

	//compute average experience of the SOM's neurons
	for(i=0;i<som_width;++i)
	{
		for(j=0;j<som_height;++j)
		{
			cell_contents* c= (cell_contents*)(som->map[i][j])->pointer;
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
}

void Self_Organizing_Neurons::sortByFitness(cell_contents* c)
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
void Self_Organizing_Neurons::removeNovel()
{
	int i,j,k;

	//decrement all the novel population
	for(i=0;i<som_width;++i)
	{
		for(j=0;j<som_height;++j)
		{
			cell_contents* c= (cell_contents*)(som->map[i][j])->pointer;
			if(c!=NULL)
			{
				for(k=0;k<novel_solutions_size;++k)
				{
					//(population[c->novel_index[k]])->decrement();
					decrement(c->novel_index[k]);
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
void Self_Organizing_Neurons::removeNovel(cell_contents* c)
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

void Self_Organizing_Neurons::decrement(int individual)
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
void Self_Organizing_Neurons::createNovelIndividuals(int i_index, int j_index, cell_contents* c)
{
	int k;

	for(k=0;k<novel_solutions_size;++k)
	{
		if(ORIGINAL_DE_OPERATOR)
		{
			if(random->uniform(0.0,1.0) < PROBABILITY_OF_USING_DE)
			{
				c->novel_index[k]= createOffspring(i_index,j_index,c,k);
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
				c->novel_index[k]= createOffspring(i_index,j_index,c);
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
int Self_Organizing_Neurons::createRandomIndividual()
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
int Self_Organizing_Neurons::createOffspring(int i_index, int j_index, cell_contents* c, int novel_index)
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

	//int r= randomIndividual();
	int r= getRandomNeighborOrRandomIndividual(i_index, j_index);
	double* r1_dna= (population[r])->dna;
	//r= randomIndividual();
	r= getRandomNeighborOrRandomIndividual(i_index, j_index);
	double* r2_dna= (population[r])->dna;
	//r= randomIndividual();
	r= getRandomNeighborOrRandomIndividual(i_index, j_index);
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
int Self_Organizing_Neurons::createOffspring(int i_index, int j_index, cell_contents* c)
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

	//int r= randomIndividual();
	int r= getRandomNeighborOrRandomIndividual(i_index, j_index);
	double* r1_dna= (population[r])->dna;
	//r= randomIndividual();
	r= getRandomNeighborOrRandomIndividual(i_index, j_index);
	double* r2_dna= (population[r])->dna;
	//r= randomIndividual();
	r= getRandomNeighborOrRandomIndividual(i_index, j_index);
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
int Self_Organizing_Neurons::randomIndividual()
{
	int random_index= random->uniform(0,population_size-1);

	//make sure that the random individual from the population has not been deleted recently (numerosity==0)
	while((population[random_index])->numerosity==0)
	{
		random_index= random->uniform(0,population_size-1);
	}

	return random_index;
}

void Self_Organizing_Neurons::clearExperience()
{
	int i,j;

	for(i=0;i<som_width;++i)
	{
		for(j=0;j<som_height;++j)
		{
			cell_contents* c= (cell_contents*)(som->map[i][j])->pointer;
			if(c!=NULL)
			{
				c->experience=0;
			}
		}
	}

}

#ifdef	PRINT_DIVERSITY
void Self_Organizing_Neurons::printAverageDiversity()
{
	int i,j,k,l;
			
	double global_diversity=0.0;
	double global_counter=0;

	for(i=0;i<som_width;++i)
	{
		for(j=0;j<som_height;++j)
		{
			cell_contents* contents= (cell_contents*)(som->map[i][j])->pointer;
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
	}

	if(global_counter!=0)
	{
		global_diversity= global_diversity/global_counter;
				
		fprintf(diversity_fp,"%f\n",global_diversity);
	}

}
#endif

void Self_Organizing_Neurons::checkDNALimits(double* dna, int dna_size)
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

void Self_Organizing_Neurons::storeAction(int i_index, int j_index, int model_index, double* observation, double* action)
{
	int i;

#ifdef	PRINT_DIVERSITY
	printAverageDiversity();
#endif	

	prev_som_x= i_index;
	prev_som_y= j_index;
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
		
void Self_Organizing_Neurons::endEpisode()
{

}

void Self_Organizing_Neurons::surrogateModelUpdate(double fitness)
{
	double output[1];
	output[0]=fitness;
	
#ifdef PRINT_SURROGATE_ERROR	
	double* approx_fitness= incremental_classifier->predict(previous_observation_action);

	fprintf(surrogate_error_fp,"%f\n",sqrt((approx_fitness[0]-fitness)*(approx_fitness[0] - fitness)));
#endif	

	//wait the SOM get a better map and the the individuals to have a more accurate fitness
	if(surrogate_counter > 1000000)
	{
		//incremental_classifier->incrementSamples(previous_observation_action, output);
	}
	else
	{
		surrogate_counter++;
	}
				

}

void Self_Organizing_Neurons::reinforcement(double fitness)
{
	int i,j;
	
	if(prev_model == -1)
	{
		return;
	}
	
	//get previous activated cell
	cell_contents* contents= (cell_contents*)(som->map[prev_som_x][prev_som_y])->pointer;
	
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

		//update the fitness of previous activated model
		if(prev_exploratory)
		{
			//Widrow-Hoff
			//printf("contents %f %d\n",contents->novel_fitness[prev_model], prev_model);
#ifdef	PROBABILISTIC_EXPERIENCE
			contents->novel_fitness[prev_model]+= widrow_coefficient*(fitness - contents->novel_fitness[prev_model]);
#endif
#ifdef  DETERMINISTIC_EXPERIENCE
			contents->novel_fitness[prev_model]+= fitness;
			contents->novel_counter[prev_model]++;
			
			if(contents->novel_counter[prev_model] ==  EXPERIENCE_PER_INDIVIDUAL)
			{
				contents->experience++;
			}
#endif			
			//printf("contents %f\n",contents->novel_fitness[prev_model]);
		}
		else
		{
			//Widrow-Hoff
			//printf("contents %f %d\n",contents->best_fitness[prev_model], prev_model);
#ifdef	PROBABILISTIC_EXPERIENCE
			contents->best_fitness[prev_model]+= widrow_coefficient*(fitness - contents->best_fitness[prev_model]);
#endif
#ifdef  DETERMINISTIC_EXPERIENCE
			contents->best_fitness[prev_model]+= fitness;
			contents->best_counter[prev_model]++;
			
			if(contents->best_counter[prev_model] ==  EXPERIENCE_PER_INDIVIDUAL)
			{
				contents->experience++;
			}
#endif			
			//printf("contents %f\n",contents->best_fitness[prev_model]);
		}
	}
		

#ifdef	PROBABILISTIC_EXPERIENCE
	contents->experience++;
	contents->accum_experience++;
#endif

	localGeneticAlgorithm(prev_som_x, prev_som_y, contents);
}

//The first time a cell wins the competition inside the SOM,
//its contents are created by this function
void Self_Organizing_Neurons::createCellContents(int i_index, int j_index)
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

	int neighbor_x= -1;
	int neighbor_y= -1;
	getMostExperientNeighbor(i_index, j_index, neighbor_x, neighbor_y);

	//printf("x %d y %d\n", neighbor_x, neighbor_y);

	//if there is at least one neighbor present
	if(neighbor_x!=-1)
	{
		cell_contents* best_experience_per_distance = (cell_contents*)(som->map[neighbor_x][neighbor_y])->pointer;
		//best individuals are chosen from the neighborhood
		for(i=0;i<best_solutions_size;++i)
		{
			contents->best_index[i]= best_experience_per_distance->best_index[i];
			(population[contents->best_index[i]])->increment();
			contents->best_fitness[i]=STARTING_BEST_FITNESS;
			contents->best_counter[i]=0;
		}
	}
	//no neighbor
	else
	{
		for(i=0;i<best_solutions_size;++i)
		{
			//contents->best_index[i]= randomIndividual();
			//contents->best_index[i]= random->uniform(0,population_size-1);
			contents->best_index[i]= createRandomIndividual();
			(population[contents->best_index[i]])->increment();
			contents->best_fitness[i]=STARTING_BEST_FITNESS;
			contents->best_counter[i]=0;
		}
	}
	
	contents->experience= 0;
	contents->accum_experience= 0;

	(som->map[i_index][j_index])->pointer= contents;
}

//return the index to a random cell in the neighborhood of the cell's position passed as parameter
int Self_Organizing_Neurons::getRandomNeighborOrRandomIndividual(int i_index, int j_index)
{
	int dx,dy;
	int neighbor_distance= DE_NEIGHBORHOOD;
	//double best_experience_per_distance=-1;

	int counter=0;
	dx= random->uniform(0,neighbor_distance*2) -neighbor_distance;
	dy= random->uniform(0,neighbor_distance*2) -neighbor_distance;
	cell_contents* contents= (cell_contents*)(som->map[i_index][j_index])->pointer;
	while((dx==0&&dy==0)||(contents==NULL&&counter<5))
	{
		dx= random->uniform(0,2) -1;
		dy= random->uniform(0,2) -1;
		contents= (cell_contents*)(som->map[i_index][j_index])->pointer;
				
		counter++;
	}

	//printf("dx %d dy %d counter %d contents %p \n",dx,dy, counter, contents);

	int solution_index= -1;

	//if we looked in the SOM neighborhood and found a good cell
	//return it, otherwise return a random invidual
	if(counter<5)
	{
		int bin= random->uniform(0,1);
		//int bin=1;

		if(bin==1)
		{
			int random_solution_from_cell = random->uniform(0, best_solutions_size-1);
			solution_index= contents->best_index[random_solution_from_cell];
		}
		else
		{
			return randomIndividual();
			
		}
	}
	else
	{
		return randomIndividual();
	}

	return solution_index;
}

//return the position of the closest SOM's cell with a cell_contents which is most experient
//Objective: 
//	-close position
//	-big experience (widely used)
void Self_Organizing_Neurons::getMostExperientNeighbor(int i_index, int j_index, int& neighbor_x, int& neighbor_y)
{
	int dx,dy;
	int neighbor_distance= MAX_NEIGHBOR_DISTANCE;
	double best_experience_per_distance=-1;
	neighbor_x=-1;
	neighbor_y=-1;

	for(dx= -neighbor_distance; dx <= neighbor_distance; ++dx)
	{
		for(dy= -neighbor_distance ; dy <= neighbor_distance ; ++dy)
		{
			int x= i_index + dx;
			int y= j_index + dy;
			double distance=  (double)chebyshevDistance(dx, dy);

			//exclude itself
			if(distance==0)
			{
				continue;
			}

			if(x > 0 && y > 0 && x < som->width && y < som->height)
			{
				cell_contents* contents= (cell_contents*)(som->map[x][y])->pointer;
				
				if(contents!=NULL)
				{
					//double neighbor_experience_per_distance= contents->accum_experience/(distance*distance);
					double neighbor_experience_per_distance= contents->experience/(distance*distance);

					if(best_experience_per_distance < neighbor_experience_per_distance)
					{
						neighbor_x= x;
						neighbor_y= y;
						best_experience_per_distance= neighbor_experience_per_distance;
					}
				}
			}
		}
	}

}

void Self_Organizing_Neurons::print()
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
int Self_Organizing_Neurons::chebyshevDistance(int dx, int dy)
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
bool Self_Organizing_Neurons::runOrNotRunGA(cell_contents* c)
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

void Self_Organizing_Neurons::printExperience()
{
	int i,j;

	for(i=0;i<som_width;++i)
	{
		for(j=0;j<som_height;++j)
		{
			cell_contents* c= (cell_contents*)(som->map[i][j])->pointer;
			if(c==NULL)
			{
				printf("%3.0f, ",-1.0);
			}
			else
			{
				printf("%3.0lf, ",c->experience);
			}
		}
		printf("\n");
	}

}

void Self_Organizing_Neurons::printSOM(const char* filename)
{
	//som->printToFile(filename);
	int i,j;

	FILE* fp = fopen(filename,"w");

	//fprintf(fp,"width height %d %d\n",width,height);
	for(i=0;i<som->width;++i)
	{
		for(j=0;j<som->height;++j)
		{
			(som->map[i][j])->printToFile(fp);
			cell_contents* c= (cell_contents*)(som->map[i][j])->pointer;
			if(c!=NULL)
			{
				fprintf(fp,"%3f \n",c->experience);
			}
			else
			{
				fprintf(fp,"0 \n");
			}
		}
	}
	fclose(fp);
}

void Self_Organizing_Neurons::printActionDistribution()
{
	int i,j,k;

	for(i=0;i<som_width;++i)
	{
		for(j=0;j<som_height;++j)
		{
			cell_contents* c= (cell_contents*)(som->map[i][j])->pointer;
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
		}
		printf("\n");
	}

}

void Self_Organizing_Neurons::printDNA()
{
	int i,j,k,l;
	
	printf("Print DNA\n");
	for(i=0;i<som_width;++i)
	{
		for(j=0;j<som_height;++j)
		{
			cell_contents* contents= (cell_contents*)(som->map[i][j])->pointer;
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
	
}

void Self_Organizing_Neurons::printCellContents(cell_contents* c)
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

void Self_Organizing_Neurons::initPrint()
{
	population_fp= fopen("population_size","w");
}

void Self_Organizing_Neurons::finishPrint()
{
	fclose(population_fp);
}

void Self_Organizing_Neurons::printPopulationSize()
{
	fprintf(population_fp,"%d\n",population_size - (int)deleted_individuals.size());
}

//return the maximum Fitness of a given cell passed as parameter
double Self_Organizing_Neurons::maxFitness(cell_contents* contents)
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

void Self_Organizing_Neurons::saveAgent(const char* filename)
{
	printf("saveAgent() not implemented\n");

}

void Self_Organizing_Neurons::loadAgent(const char* filename)
{
	printf("loadAgent() not implemented\n");
}

