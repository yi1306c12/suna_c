
#include"NEAT_Interface.h"

NEAT_Interface::NEAT_Interface(Random* random, const char* filename_with_parameters)
{
	this->random= random;
  
	NEAT::Population *p=0;
  
	NEAT::load_neat_params(filename_with_parameters,SHOW_NEAT_PARAMETERS);

    	pop=0;
	champ_net=NULL;
	step_counter=0;

	//only one experiment
	expcount=0;
	if(NEAT::num_runs != 1)
	{
		printf("ERROR: NEAT::num_runs should be 1.\n");
		exit(1);
	}

	gen=1;

	memset (runs, 0, NEAT::num_runs * sizeof(int));

	srand((unsigned) time( NULL ) );
}

NEAT_Interface::~NEAT_Interface()
{
	free(action);
}

void NEAT_Interface::init(int number_of_observation_vars, int number_of_action_vars)
{
	this->number_of_observation_vars= number_of_observation_vars;
	this->number_of_action_vars= number_of_action_vars;

	action= (double*) malloc(number_of_action_vars*sizeof(double));
	modified_input= (double*) malloc((number_of_observation_vars+1)*sizeof(double));

	
	//------------ finish the initialization ----------	
	
	createInitialGene("initial_gene",number_of_observation_vars,number_of_action_vars);

	ifstream iFile("initial_gene",ios::in);
	//ifstream iFile("agents/neat/pole1startgenes",ios::in);
	//ifstream iFile(filename_initial_genes,ios::in);

//	cout<<"START SINGLE POLE BALANCING EVOLUTION"<<endl;

//	cout<<"Reading in the start genome"<<endl;
	//Read in the start Genome
	iFile>>curword;
	iFile>>id;
//	cout<<"Reading in Genome id "<<id<<endl;
	start_genome=new Genome(id,iFile);
	iFile.close();
    
//	cout<<"EXPERIMENT #"<<expcount<<endl;

//	cout<<"Start Genome: "<<start_genome<<endl;

	//Spawn the Population
//	cout<<"Spawning Population off Genome"<<endl;

	pop=new Population(start_genome,NEAT::pop_size);

//	cout<<"Verifying Spawned Pop"<<endl;
	pop->verify();
	
	curorg=(pop->organisms).begin();

	//first champion	
	champ_net=(*curorg)->net;
}

void NEAT_Interface::step(double* observation, double reward)
{
	Organism *org= *curorg;
	Network *net;
	vector<NNode*>::iterator out_iter;

	//update fitness	
	org->fitness+= reward;

	//int numnodes;  	// Used to figure out how many nodes	should be visited during activation 

	//int MAX_STEPS=120000;
	//int MAX_STEPS=100000;

	net=org->net;
	//numnodes=((org->gnome)->nodes).size();

	//Try to balance a pole now
	//org->fitness = go_cart(net,MAX_STEPS,thresh);

#ifdef USE_INITIAL_BIAS	
	for(int i=0;i<number_of_observation_vars;++i)
	{
		modified_input[i]=observation[i];
	}
	modified_input[number_of_observation_vars]=NEAT_INITIAL_BIAS;
#else
	for(int i=0;i<number_of_observation_vars;++i)
	{
		modified_input[i]=observation[i];
	}
#endif	
	
	net->load_sensors(modified_input);

	//Activate the net
	//If it loops, exit returning only fitness of 1 step
	if(!(net->activate()))
	{	
	
		out_iter=net->outputs.begin();
		for(int i=0; i<number_of_action_vars; ++i)
		{
			action[i]=0;
		}
		
		//org->fitness= EXTREME_NEGATIVE_REWARD;

		return;
	}

	out_iter=net->outputs.begin();
	for(int i=0; i<number_of_action_vars; ++i)
	{
		action[i]=(*out_iter)->activation;
		++out_iter;
	}



//#ifndef NO_SCREEN_OUT
//	cout<<"Org "<<(org->gnome)->genome_id<<" fitness: "<<org->fitness<<endl;
//#endif

	//Decide if its a winner
/*	if(org->fitness>=MAX_STEPS) 
	{ 
		org->winner=true;
		return true;
	}
	else 
	{
		org->winner=false;
		return false;
	}
*/
		    
/*
      for (gen=1;gen<=gens;gen++) 
      {
	cout<<"Generation "<<gen<<endl;
	
	fnamebuf=new ostringstream();
	(*fnamebuf)<<"gen_"<<gen<<ends;  //needs end marker

#ifndef NO_SCREEN_OUT
	cout<<"name of fname: "<<fnamebuf->str()<<endl;
#endif	

	char temp[50];
        sprintf (temp, "gen_%d", gen);




	status=pole1_epoch(pop,gen,temp);
	//status=(pole1_epoch(pop,gen,fnamebuf->str()));
	
	if (status) 
	{
	  runs[expcount]=status;
	  gen=gens+1;
	}




	fnamebuf->clear();
	delete fnamebuf;
	
      }

	delete pop;

	totalevals=0;
	samples=0;
    for(expcount=0;expcount<NEAT::num_runs;expcount++) 
    {
      cout<<runs[expcount]<<endl;
      if (runs[expcount]>0)
      {
        totalevals+=runs[expcount];
        samples++;
      }
    }
*/
	//cout<<"Failures: "<<(NEAT::num_runs-samples)<<" out of "<<NEAT::num_runs<<" runs"<<endl;
	//cout<<"Average evals: "<<(samples>0 ? (double)totalevals/samples : 0)<<endl;

	//return pop;
		
}

void NEAT_Interface::print()
{

}
		
void NEAT_Interface::endEpisode(double reward)
{
	Organism *org= *curorg;
	//org->fitness+= reward;
	
	//additional fitness added to avoid negative fitness values
	//for some reason it seems that NEAT does not work with negative fitness values
	org->fitness+= reward+NEAT_ADDITIONAL_FITNESS_HACK;

	step_counter++;

	//average over tested episodes 
	if(step_counter >= EPISODES_PER_INDIVIDUAL)
	{
		org->fitness/= (double) step_counter;
		step_counter=0;
	}
	else
	{
		return;
	}
	
	//next organism	
	curorg++;
	
	if(curorg!=(pop->organisms).end()) 
	{
		(*curorg)->fitness= 0;
		
		//do not evolve
		return;
	}

	//----------------------------  Evolve -----------------------------//

	//findBestIndividual();
  
	vector<Species*>::iterator curspecies;
	//Average and max their fitnesses for dumping to file and snapshot
  	for(curspecies=(pop->species).begin();curspecies!=(pop->species).end();++curspecies) 
	{
		//This experiment control routine issues commands to collect ave
		//and max fitness, as opposed to having the snapshot do it, 
		//because this allows flexibility in terms of what time
		//to observe fitnesses at

		(*curspecies)->compute_average_fitness();
		(*curspecies)->compute_max_fitness();
	}

/*
  //Only print to file every print_every generations
  if  (win||
       ((generation%(NEAT::print_every))==0))
    pop->print_to_file_by_species(filename);

  if (win) {
    for(curorg=(pop->organisms).begin();curorg!=(pop->organisms).end();++curorg) {
      if ((*curorg)->winner) {
	winnernum=((*curorg)->gnome)->genome_id;
	cout<<"WINNER IS #"<<((*curorg)->gnome)->genome_id<<endl;
      }
    }    
  }
*/
	//Create the next generation
	pop->epoch(gen);
	
	//findBestIndividual();
	
	//set the next organism
	curorg=(pop->organisms).begin();
	
	(*curorg)->fitness= 0;
	
	//champ=(*curorg);

	gen++;

}

double NEAT_Interface::stepBestAction(double* observation)
{
	if(champ_net==NULL)
	{
		printf("Champion not set yet\n");
		return 0;
	}
	
	//Network *net;
	vector<NNode*>::iterator out_iter;

	/*int numnodes;   Used to figure out how many nodes
	should be visited during activation */
	//int thresh;  // How many visits will be allowed before giving up (for loop detection) 

	//use the network from the best organism
	//net=champ->net;
	//numnodes=((champ->gnome)->nodes).size();
	//thresh=numnodes*2;  //Max number of visits allowed per activation

	//Try to balance a pole now
	//org->fitness = go_cart(net,MAX_STEPS,thresh);

#ifdef USE_INITIAL_BIAS	
	for(int i=0;i<number_of_observation_vars;++i)
	{
		modified_input[i]=observation[i];
	}
	modified_input[number_of_observation_vars]=NEAT_INITIAL_BIAS;
	champ_net->load_sensors(modified_input);
#else
	champ_net->load_sensors(observation);
#endif	

	//Activate the net
	//If it loops, exit returning only fitness of 1 step
	if(!(champ_net->activate()))
	{	
		out_iter=champ_net->outputs.begin();
		for(int i=0; i<number_of_action_vars; ++i)
		{
			action[i]=0;
		}

		return 0;
	}

	out_iter=champ_net->outputs.begin();
	for(int i=0; i<number_of_action_vars; ++i)
	{
		action[i]=(*out_iter)->activation;
		++out_iter;
	}

	return 0;
}

void NEAT_Interface::saveAgent(const char* filename)
{
	//printf("saveAgent() not implemented\n");

}

void NEAT_Interface::loadAgent(const char* filename)
{
	//printf("loadAgent() not implemented\n");
}

void NEAT_Interface::findBestIndividual()
{
	vector<Organism*>::iterator torg;

	torg=(pop->organisms).begin();
	Organism* current_champ=(*torg);
	double champ_fitness=current_champ->fitness;
	double avg_fitness=0.0;
	double counter=0;
	for(;torg!=(pop->organisms).end();++torg) 
	{
		if(((*torg)->fitness) > champ_fitness) 
		{
			current_champ=(*torg);
			champ_fitness=current_champ->fitness;
			//champgenes=champ->gnome->genes.size();
			//champnodes=champ->gnome->nodes.size();
			//winnernum=champ->gnome->genome_id;
	
		}
			
		//printf("fitness: %f\n", (*torg)->fitness);
		avg_fitness+= (*torg)->fitness;
		counter++;
	}
	avg_fitness/=counter;

	//delete past champion
	//delete champ_net;
	
	//copy the current champion
	//champ_net= new Network(*(current_champ->net));
	

	printf("%d %.2f %.2f\n", gen,champ_fitness, avg_fitness);

	//exit(1);
}

void NEAT_Interface::createInitialGene(const char* filename, int number_of_inputs, int number_of_outputs)
{
	FILE* fp= fopen(filename,"w");
	
	fprintf(fp,"genomestart 1\n");
	
	fprintf(fp,"trait 1 0.1 0 0 0 0 0 0 0\n");
	fprintf(fp,"trait 2 0.2 0 0 0 0 0 0 0\n");
	fprintf(fp,"trait 3 0.3 0 0 0 0 0 0 0\n");
	
	//node encoding:
	//node id_number trait_number node_type(1-input or 0-output) genetic_node_label(1-input, 2-output, 3-bias)

	int i;
	for(i=0 ; i< number_of_inputs ; ++i)
	{
		fprintf(fp,"node %d 0 1 1\n",i+1);
	}
	
	for(; i< number_of_inputs + number_of_outputs ; ++i)
	{
		fprintf(fp,"node %d 0 0 2\n",i+1);
	}

#ifdef USE_INITIAL_BIAS	
	//add a bias
	fprintf(fp,"node %d 0 1 3\n",i+1);
#endif
	//gene encoding:
	//gene trait_number connection_in_node connection_out_node weight_of_the_connection recur_flag innovation_number mutation_number(same as weight) enable_bit
	
	int innovation=1;
	for(i=0; i< number_of_inputs; ++i)
	{
		for(int j=0; j<number_of_outputs ; ++j)
		{
			fprintf(fp,"gene 1 %d %d 0.0 0 %d 0 1\n",i+1, number_of_inputs+j+1,innovation);
		
			innovation++;
		}

	}
		
#ifdef USE_INITIAL_BIAS	
	for(int j=0; j<number_of_outputs ; ++j)
	{
		fprintf(fp,"gene 1 %d %d 0.0 0 %d 0 1\n",number_of_inputs+number_of_outputs+1, number_of_inputs+j+1,innovation);
	
		innovation++;
	}
#endif


	fprintf(fp,"genomeend 1\n");
	
	fclose(fp);

}
