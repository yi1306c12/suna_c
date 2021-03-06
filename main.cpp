
#include"stdio.h"
#include"stdlib.h"

//for random seed
#include<unistd.h>//getpid
#include<time.h>//clock

//for experiment time
#include<ctime>//strftime

//agents
#include"agents/Unified_Neural_Model.h"

//environments
//#include"environments/Frog_Fly.h"
#include"environments/Function_Approximation.h"
#include"environments/Continuous_Empty_Maze.h"
#include"environments/Single_Cart_Pole.h"
#include"environments/Pendulum_Swing_Up.h"
#include"environments/Double_Cart_Pole.h"
#include"environments/Mountain_Car.h"
#include"environments/Go.h"
#include"environments/Multiplexer.h"
#include"environments/General_Labyrinth.h"
#include"environments/Copy.h"
#include"environments/Caesar_Cipher.h"
#include"environments/Caesar_Cipher_multiplicatekey.hpp"
#include"environments/Randomized_Double_Cart_Pole.hpp"
#include"environments/Randomized_Pendulum_Swing_Up.hpp"
#include"parameters.h"

FILE* main_log_file;

double computeAverage(double* last_rewards, int counter)
{
	int k;

	if(counter>100)
	{
		counter=100;
	}

	double avg_rewards=0.0;
	//printf("AAA\n");
	for(k=0;k<counter;++k)
	{
		//printf("%f\n",last_rewards[k]);
		avg_rewards+= last_rewards[k];
	}
	avg_rewards= avg_rewards/(double)counter;

	return avg_rewards;
}

void setFeatures(Reinforcement_Environment* env)
{

#ifdef SET_NORMALIZED_INPUT
	bool feature_available;

	feature_available= env->set(NORMALIZED_OBSERVATION);

	if(feature_available == false)
	{
		printf("NORMALIZED_OBSERVATION feature not available\n");
		exit(1);
	}
	else
	{
		fprintf(main_log_file,"Normalized Observation enabled\n");
	}
#endif

#ifdef SET_NORMALIZED_OUTPUT
	bool feature_available;

	feature_available= env->set(NORMALIZED_ACTION);

	if(feature_available == false)
	{
		printf("NORMALIZED_ACTION feature not available\n");
		exit(1);
	}
	else
	{
		fprintf(main_log_file,"Normalized Action enabled\n");
	}
#endif

}

int main(int const argc, char const * argv[])
{
	int trials_to_change_maze_states= 10000;
	int i;
	
	char const * mainlog_filename = argv[1];
	char const * network_filename = argv[2];
	char const * DNA_filename = argv[3];

	main_log_file= fopen(mainlog_filename,"w");

	int unsigned const randseed = static_cast<unsigned int>(clock())+static_cast<unsigned int>(getpid());
	

	Random* random= new State_of_Art_Random(randseed);

	//Reinforcement_Environment* env= new Frog_Fly(1);
//	Reinforcement_Environment* env= new Continuous_Empty_Maze();
	//Reinforcement_Environment* env= new General_Labyrinth(trials_to_change_maze_states, random);
	//Reinforcement_Environment* env= new Mountain_Car(random);
	//Reinforcement_Environment* env= new Function_Approximation(random,1000,false);
	//Reinforcement_Environment* env= new Single_Cart_Pole(random);
//	Reinforcement_Environment* env= new Caesar_Cipher(random);
//	Reinforcement_Environment* env= new Caesar_Cipher_multiplicatekey(random, 2);
//	Reinforcement_Environment* env= new Double_Cart_Pole(random);
//	Reinforcement_Environment* env= new Multiplexer(2,4,random);
//	Reinforcement_Environment* env= new Copy(4,4,20,random);
	//Reinforcement_Environment* env= new Pendulum_Swing_Up(random);
	//Reinforcement_Environment* env= new Go(random, BOARDSIZE);

	//Reinforcement_Environment* env = new Randomized_Double_Cart_Pole(random, 0., 0.01);
	Reinforcement_Environment* env = new Randomized_Pendulum_Swing_Up(random, 0., 0.01);

	//Unified_Neural_Model* agent= new Mysterious_Agent();
	//Unified_Neural_Model* agent= new Neural_XCSF(population_size, number_of_hidden, layers);
	//Unified_Neural_Model* agent= new XCSF(population_size);
	//Unified_Neural_Model* agent= new Dummy(env);
	Unified_Neural_Model* agent= new Unified_Neural_Model(random);
	//Unified_Neural_Model* agent= new NEAT_Interface(random, NEAT_PARAMETERS);
	//Unified_Neural_Model* agent= new Self_Organizing_Neurons(SOM_SIZE,SOM_SIZE, random);
	//Unified_Neural_Model* agent= new Novelty_Organizing_Neurons(random);
	//Unified_Neural_Model* agent= new NOTC(random);
	//Unified_Neural_Model* agent= new Cacla(random);

	setFeatures(env);

	//Self_Organizing_Neurons* b= (Self_Organizing_Neurons*)agent;

	//print max accumulated reward seen in N trials, the N trials is given by trial_frequency_to_print
	bool print_max_accum_reward_in_n_trials= true;
	int trial_frequency_to_print= 100;
	double max_accum_reward=0;
	bool was_initialized=false;	//tells if the max_accum_reward was initialized

	bool print_reward=false;
	bool print_step=false;
	bool print_average=false;
	//bool print_accumulated_reward=true;
	bool print_agent_information=false;

	//int trials=100000
	//int trials=300000;
	int trials=1000000;
	//int trials=200;
	//int trials=500;
	//int trials=100000;

	int number_of_observation_vars;
	int number_of_action_vars;

	env->start(number_of_observation_vars, number_of_action_vars);
	agent->init(number_of_observation_vars, number_of_action_vars);

	//starting reward
	double reward= env->step(NULL);
	double step_counter=1;

	//agent->print();

	double last_rewards[100];
	int counter=0;
	double avg_rewards;

	//environment log
	time_t rawtime;
	char buffer[80];
	time(&rawtime);
	struct tm * timeinfo = localtime(&rawtime);
	strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
	fprintf(main_log_file, "start time:\t%s\n", buffer);
	fprintf(main_log_file, "random_seed:\t%u\n", randseed);
	fprintf(main_log_file, "trials:\t%d\n", trials);

	for(i=env->trial;i<trials;)
	{
		double accum_reward=reward;
		//do one trial (multiple steps until the environment finish the trial or the trial reaches its MAX_STEPS)
		while(env->trial==i && step_counter <= env->MAX_STEPS)
		{

			agent->step(env->observation, reward);

			reward= env->step(agent->action);

			accum_reward+= reward;

			if(print_reward)
			{
				last_rewards[counter%100]=reward;
				counter++;

				if(print_average)
				{
					avg_rewards= computeAverage(last_rewards, counter);
					printf("%d %f\n",i, avg_rewards);
				}
				else
				{
					printf("%d %f\n",i, reward);
				}


			}

			step_counter++;

		}

#ifdef TERMINATE_IF_MAX_STEPS_REACHED
		//end evolution when the MAX_STEPS is reached
		if(step_counter > env->MAX_STEPS)
		{
			i=trials;
		}
#endif

		//update the max_accum_reward and print
		if(print_max_accum_reward_in_n_trials)
		{
			if(was_initialized==false)
			{
				was_initialized=true;
				max_accum_reward= accum_reward;
			}
			else
			{
				if(max_accum_reward < accum_reward)
				{
					max_accum_reward= accum_reward;
				}
			}

			if(i%trial_frequency_to_print==0)
			{
				printf("%d %f\n",i, max_accum_reward);

				max_accum_reward=0;
				was_initialized=false;

			}
		}

		agent->endEpisode(reward);


		//if env->trial is the same as i, it means that the internal state of the environment has not changed
		//then it needs a restart to begin a new trial
		if(env->trial==i)
		{
			reward= env->restart();
		}
		else
		{
			reward= env->step(NULL);
		}

		//print the number of steps used in the last trial
		if(print_step)
		{
			last_rewards[counter%100]=step_counter;
			counter++;

			if(print_average)
			{
				avg_rewards= computeAverage(last_rewards, counter);
				printf("%d %f\n",i, avg_rewards);
			}
			else
			{
				printf("%d %f\n",i, step_counter);
			}
		}

		if(print_agent_information==true)
		{
			agent->print();
		}

		step_counter=1;

		i++;

#ifdef STOP_REWARD
		if(max_accum_reward > STOP_REWARD)
		{
			printf("%d %f\n",i,max_accum_reward);
			break;
		}
#endif

	}

	agent->saveGraph_bestAgent(network_filename);
	agent->saveAgent(DNA_filename);



	//printf("reward average %f\n",reward_sum/(double)trials);
	//printf("step average %f\n",step_sum/(double)trials);

	fclose(main_log_file);

	return 0;
}
