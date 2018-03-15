#include"stdio.h"
#include"stdlib.h"

#include"agents/Unified_Neural_Model.h"
#include"environments/Gym_Wrapper.hpp"
#include"parameters.h"

int main()
{
	//int trials_to_change_maze_states= 10000;

	Random* random= new State_of_Art_Random(time(NULL));
	
	Reinforcement_Environment* env = new Gym_Wrapper("BipedalWalker-v2");
	Reinforcement_Agent* agent= new Unified_Neural_Model(random);

	//print max accumulated reward seen in N trials, the N trials is given by trial_frequency_to_print
	bool print_max_accum_reward_in_n_trials= true;
	int trial_frequency_to_print= 100;
	double max_accum_reward=0;
	bool was_initialized=false;	//tells if the max_accum_reward was initialized

	//bool print_accumulated_reward=true;

	//int trials=100000
	int trials=300000;		
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
		
		
	for(int i=env->trial;i<trials;)
	{
		double accum_reward=reward;
		//do one trial (multiple steps until the environment finish the trial or the trial reaches its MAX_STEPS)
		while(env->trial==i && step_counter <= env->MAX_STEPS)
		{

			agent->step(env->observation, reward);

			reward= env->step(agent->action);		
		
			accum_reward+= reward;
		
			step_counter++;

		}

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

	agent->saveAgent("dna_best_individual");
	


	//printf("reward average %f\n",reward_sum/(double)trials);
	//printf("step average %f\n",step_sum/(double)trials);
	

	return 0;
}

