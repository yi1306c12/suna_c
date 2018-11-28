
#include"Discrete_Labyrinth.h"

Discrete_Labyrinth::Discrete_Labyrinth(int trials_to_change_maze_states, Random* random)
{
	bool has_title=false;
	current_maze_state= 0;
	this->trials_to_change_maze_states= trials_to_change_maze_states;
	
	int i;
	MAX_STEPS= MAX_MAZE_STEPS;

	/////////////////////////////////////
	//////////  load maze(s)  ///////////
	
	FILE* fp= fopen(PATH_TO_MAZE_CONFIG,"r");
	fscanf(fp,"%d",&number_of_maze_states);
	map= (double***)malloc(sizeof(double**)*number_of_maze_states);
	size_x= (int*)malloc(sizeof(int)*number_of_maze_states);
	size_y= (int*)malloc(sizeof(int)*number_of_maze_states);
	
	for(i=0;i<number_of_maze_states;++i)
	{
		//load maze
		char maze_filename[FILENAME_SIZE];
		fscanf(fp,"%s",maze_filename);

		//read maze
		map[i] = readCSV(maze_filename, size_x[i], size_y[i], has_title);
	
		convertMap(i);
	}
	fclose(fp);

	////////////////////////////////////
	
	//printf("aaaaaa %d\n",number_of_maze_states);
	//printf("aaaaaa %s\n",csv_filename[0]);
	//exit(1);

	this->noisy_input= ENVIRONMENT_NOISY_INPUT;
	

	max_velocity_per_step= 1.0;

	//usual reward given to the system, it means nothing important.
	default_reward=-1;

	this->random= random;

	print_trajectory=false;
	trial=0;

	if(print_trajectory)
	{
		char filename[64];
		sprintf(filename, "trajectories/trajectory_%d",trial);
		fp= fopen(filename,"w");
	}

}

Discrete_Labyrinth::~Discrete_Labyrinth()
{
	free(observation);

}

	
//set the number of observable vars and number of actions
void Discrete_Labyrinth::start(int &number_of_observation_vars, int& number_of_action_vars)
{
	number_of_observation_vars= 3;
	number_of_action_vars=2;

	this->number_of_observation_vars= number_of_observation_vars;
	this->number_of_action_vars= number_of_action_vars;

	//random= new State_of_Art_Random(time(NULL));


	//allocate observation
	observation= (double*) malloc(number_of_observation_vars*sizeof(double));
	
	//random initial state
	x_pos = random->uniform(0.0,(double)size_x[current_maze_state]);
	y_pos = random->uniform(0.0,(double)size_y[current_maze_state]);
	while(map[current_maze_state][(int)x_pos][(int)y_pos] == 3)
	{
		x_pos = random->uniform(0.0,(double)size_x[current_maze_state]);
		y_pos = random->uniform(0.0,(double)size_y[current_maze_state]);
	}
	
	if(noisy_input)
	{
		observation[0]= x_pos +random->uniform(-(double)0.05*x_pos,(double)x_pos*0.05);
		observation[1]= y_pos +random->uniform(-(double)0.05*y_pos,(double)y_pos*0.05);
		observation[2]= default_reward;
	}
	else
	{
		observation[0]= x_pos;
		observation[1]= y_pos;
		observation[2]= default_reward;
	}

	step_counter=0;
}
	
//return a reward
//and update environment's variable
double Discrete_Labyrinth::step(double* action)
{
	int i;

	////////////////// Initial Reward ///////////////////
	
	//set the value of the initial reward
	if(action==NULL)
	{
		return default_reward;
	}

	////////////////// Return Reward ///////////////////

	double reward= default_reward;

	//check for out of bound action (the velocity is limited)
	for(i=0;i < number_of_action_vars;++i)
	{
		if(action[i] > max_velocity_per_step)
		{
			action[i]= max_velocity_per_step;
		}
		if(action[i] < -max_velocity_per_step)
		{
			action[i]= -max_velocity_per_step;
		}
	}

	//update position
	x_pos+= action[0];
	y_pos+= action[1];

	//check for out of bound states (the maze is closed)
	if(x_pos < 0.0)
	{
		x_pos=0.0;
	}
	if(x_pos > size_x[current_maze_state]-1)
	{
		x_pos= size_x[current_maze_state]-1;
	}
	if(y_pos < 0.0)
	{
		y_pos=0.0;
	}
	if(y_pos > size_y[current_maze_state]-1)
	{
		y_pos= size_y[current_maze_state]-1;
	}

	//discrete case
	if(map[current_maze_state][(int)x_pos][(int)y_pos] == 3)
	{
		/*double initial_x = x_pos - action[0];
		double initial_y = y_pos - action[1];
		
		int diff_x= (int)x_pos - (int)initial_x;
		int diff_y= (int)y_pos - (int)initial_y;
		
		if(diff_x != 0 && diff_y != 0)
		{
			x_pos-= action[0];
			y_pos-= action[1];
		}
		else
		{
			if(diff_x != 0)
			{
				x_pos-= action[0];
			}
			else
			{
				y_pos-= action[1];
			}
		}
		*/
			
		x_pos-= action[0];
		y_pos-= action[1];
		
		reward= -2000.0;
		
		//set the current observation
		if(noisy_input)
		{
			observation[0]= x_pos +random->uniform(-(double)0.05*x_pos,(double)x_pos*0.05);
			observation[1]= y_pos +random->uniform(-(double)0.05*y_pos,(double)y_pos*0.05);
			observation[2]= reward;
		}
		else
		{
			observation[0]= x_pos;
			observation[1]= y_pos;
			observation[2]= reward;
		}


		return reward;
	}

	if(map[current_maze_state][(int)x_pos][(int)y_pos] == 8)
	{
		reward= 10000.0;
	
		//random initial state
		restart();
	}
	else
	{
		reward= default_reward;
	}

	//Compute reward
/*	if(x_pos >= target_x && y_pos >= target_y)
	{
		reward= 1000.0 - (double)step_counter*0.001;
	
		//random initial state
		x_pos=random->uniform(0.0,1.0);
		y_pos=random->uniform(0.0,1.0);

		step_counter=0;
	}
	else
	{
		reward= 0.0;
	}
*/


	/////////////// Update observation///////////////////////
	
	//set the current observation
	if(noisy_input)
	{
		observation[0]= x_pos +random->uniform(-(double)0.05*x_pos,(double)x_pos*0.05);
		observation[1]= y_pos +random->uniform(-(double)0.05*y_pos,(double)y_pos*0.05);
		observation[2]= reward;
	}
	else
	{
		observation[0]= x_pos;
		observation[1]= y_pos;
		observation[2]= reward;
	}

	
	if(print_trajectory)
	{
		printTrajectory(fp, observation, number_of_observation_vars);
	}

	/////////////////////////////////////////////////////

	step_counter++;

	return reward;
	
}

//restart maze problem: set the individual position at a random initial position
double Discrete_Labyrinth::restart()
{
	step_counter=0;
	trial++;

	//if there are more than one maze (dynamic maze with many states)
	if(number_of_maze_states > 1)
	{
		//test if the trials are a multiple of the periodic number of trials
		//necessary to change between maze states
		if(trial%trials_to_change_maze_states==0)
		{
			current_maze_state++;
			current_maze_state=current_maze_state%number_of_maze_states;
			//printf("changed_maze states: current %d\n",current_maze_state);
		}
	}

	//random initial state
	x_pos = random->uniform(0.0,(double)size_x[current_maze_state]);
	y_pos = random->uniform(0.0,(double)size_y[current_maze_state]);
	while(map[current_maze_state][(int)x_pos][(int)y_pos] == 3)
	{
		x_pos = random->uniform(0.0,(double)size_x[current_maze_state]);
		y_pos = random->uniform(0.0,(double)size_y[current_maze_state]);
	}

	//set the current observation
	if(noisy_input)
	{
		observation[0]= x_pos +random->uniform(-(double)0.05*x_pos,(double)x_pos*0.05);
		observation[1]= y_pos +random->uniform(-(double)0.05*y_pos,(double)y_pos*0.05);
		observation[2]= default_reward;
	}
	else
	{
		observation[0]= x_pos;
		observation[1]= y_pos;
		observation[2]= default_reward;
	}
		
	
	if(print_trajectory)
	{
		fclose(fp);
		char filename[64];
		sprintf(filename, "trajectories/trajectory_%d",trial);
		fp= fopen(filename,"w");
	}

	//return -1000.0;
	return default_reward;
}

//print the maze status
//1 - individual
//2 - obstacle
//8 - goal
//0 - empty space
void Discrete_Labyrinth::print()
{	
	//set the agent position
	int tmp= map[current_maze_state][(int)x_pos][(int)y_pos];
	map[current_maze_state][(int)x_pos][(int)y_pos]= 1;

	//print
	printMatrix(map[current_maze_state], size_x[current_maze_state], size_y[current_maze_state]);
	
	map[current_maze_state][(int)x_pos][(int)y_pos]= tmp;
}

void Discrete_Labyrinth::printAgentBehavior(Reinforcement_Agent* agent)
{
	int i,j,k;

	int iteractions=100;
	
	
	FILE* fpf;
	
	fpf= fopen("fitness_table","w");

	for(i=0;i<size_x[current_maze_state];++i)
	{
		for(j=0;j<size_y[current_maze_state];++j)
		{
			double fitness_sum=0;
			for(k=0;k<iteractions;++k)
			{
				double state[2];
				state[0]= i + random->uniform(0.0,1.0);
				state[1]= j + random->uniform(0.0,1.0);
				
				fitness_sum += agent->stepBestAction(state);
			}
			double fitness_avg= fitness_sum/(double)iteractions;
			fprintf(fpf,"%f ",fitness_avg);
			//printf("%f ",map[i][j]);
		}
		fprintf(fpf,"\n");
	}

	//plot the action

	FILE* fpx;
	FILE* fpy;
	
	//THE INVERSION OF X PER Y IS CORRECT! 
	//Because the x and y axis are inverted inside the program.
	fpy= fopen("action_x","w");
	fpx= fopen("action_y","w");

	for(i=0;i<size_x[current_maze_state];++i)
	{
		for(j=0;j<size_y[current_maze_state];++j)
		{
			double action_x=0;
			double action_y=0;
			for(k=0;k<iteractions;++k)
			{
				double state[2];
				state[0]= i + random->uniform(0.0,1.0);
				state[1]= j + random->uniform(0.0,1.0);
				
				agent->stepBestAction(state);

				double* action= agent->action;
				
				action_x+= action[0];
				action_y+= action[1];
			}
			double action_x_avg= action_x/(double)iteractions;
			double action_y_avg= action_y/(double)iteractions;
			//fprintf("x%fy%f(%d) ",action_x_avg, action_y_avg,(int)map[i][j]);
			fprintf(fpx, "%f ",action_x_avg);
			fprintf(fpy, "%f ",action_y_avg);
			//printf("%f ",map[i][j]);
		}
		fprintf(fpx,"\n");
		fprintf(fpy,"\n");
	}

	fclose(fpx);
	fclose(fpy);
	fclose(fpf);
}

void Discrete_Labyrinth::convertMap(int state)
{
	//invert coordinates of the map's matrix:
	//
	//EXAMPLE:
	//Written matrix:
	//
	//2 8 5								
	//1 6 7				
	//0 1 2			(x=0, y=0) 0
	//
	//Read matrix:
	//
	//0 1 2
	//1 6 7
	//2 8 5			(x=0, y=0) 2
	//
	//Inverted matrix (transform back to the original):
	//NOTE that the x,y coordinates are still inverted, i.e., x= y and y= x !!!!!!!!!!!!!
	//This happens because the usual map[x][y] should be map[y][x], but that may bring some CONFUSIONS!
	//Better to invert the coordinates on the graphs only...
	//
	//2 8 5
	//1 6 7
	//0 1 2
	// 
	///////////////
	int i;
	int x_mirror= size_x[current_maze_state]-1;
	int x=0;
	for(i=0;i<size_y[current_maze_state];++i)
	{
		for(x=0,x_mirror=size_x[current_maze_state]-1; x < x_mirror  ;--x_mirror,++x)
		{
			double tmp = map[state][x][i];
			map[state][x][i]=map[state][x_mirror][i];
			map[state][x_mirror][i]=  tmp;
		}
	}
}

void Discrete_Labyrinth::printTrajectory(FILE* fp, double* observation, int size)
{
	int i;

	for(i=0;i<size;++i)
	{
		fprintf(fp,"%f ",observation[i]);
	}
	fprintf(fp,"\n");
}
