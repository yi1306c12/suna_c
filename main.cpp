
#include"stdio.h"
#include"stdlib.h"

//agents
#include"agents/Unified_Neural_Model.h"

//environments
#include"environments/Function_Approximation.h"
#include"environments/Single_Cart_Pole.h"
#include"environments/Double_Cart_Pole.h"
#include"environments/Mountain_Car.h"
#include"environments/Multiplexer.h"
#include"environments/Pendulum_Swing_Up.h"
#include"environments/Count_Minority.h"

#include"parameters.h"

#include<limits>

int const trial_frequency_to_print= 100;

int main()
{

	Random* random= new State_of_Art_Random(time(NULL));
	
	//Reinforcement_Environment* env= new Mountain_Car(random);
	//Reinforcement_Environment* env= new Function_Approximation(random,1000,false);
	//Reinforcement_Environment* env= new Single_Cart_Pole(random);
	Reinforcement_Environment* env= new Double_Cart_Pole(random);
	//Reinforcement_Environment* env= new Multiplexer(3,8,random);
	//Reinforcement_Environment* env= new Pendulum_Swing_Up(random);
	//Reinforcement_Environment* env = new Count_Minority(random);
	

	Reinforcement_Agent* agent= new Unified_Neural_Model(random);
	//print max accumulated reward seen in N trials, the N trials is given by trial_frequency_to_print
	double max_accum_reward=0;
	int trials=300000;
	
	int number_of_observation_vars;
	int number_of_action_vars;
	env->start(number_of_observation_vars, number_of_action_vars);
	agent->init(number_of_observation_vars, number_of_action_vars);
	
	//starting reward 
	double reward= env->step(NULL);
	double step_counter=1;
	
	for(int i=env->trial; i<trials;)
	{
		double accum_reward=reward;
		//do one trial (multiple steps until the environment finish the trial or the trial reaches its MAX_STEPS)
		while(env->trial==i && step_counter<=env->MAX_STEPS)
		{
			agent->step(env->observation, reward);
			reward= env->step(agent->action);
			accum_reward+= reward;
			step_counter++;
		}
	
		//update the max_accum_reward and print		
		if(max_accum_reward < accum_reward)
		{
			max_accum_reward= accum_reward;
		}

#ifdef STOP_REWARD
		if(accum_reward > STOP_REWARD)
		{
			printf("%d %f\n",i,max_accum_reward);
			break;
		}
#endif


		if(i%trial_frequency_to_print==0)
		{	
			printf("%d %f\n",i, max_accum_reward);
			max_accum_reward=std::numeric_limits<float>::min();
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
	}

	agent->saveAgent("dna_best_individual");
	return 0;
}