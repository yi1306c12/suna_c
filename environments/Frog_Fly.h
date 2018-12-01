
#ifndef FROG_FLY
#define FROG_FLY

#include"Reinforcement_Environment.h"
#include"random/State_of_Art_Random.h"
#include"stdlib.h"
#include"stdio.h"
#include"time.h"
#include"math.h"

class Frog_Fly : public Reinforcement_Environment
{
	public:

		Frog_Fly(bool usual_version);
		~Frog_Fly();
		
		//All Reinforcement Problems have the observartion variable, although it is not declared here!
		//double* observation;	
		//int number_of_observation_vars;
		//int number_of_action_vars;
		
		bool usual_version;
		
		Random* random;

		double distance_from_fly;

		//Implementing the Reinforcement Problem Interface
		void start(int &number_of_observation_vars, int& number_of_action_vars);
		double step(double* action);
		double restart();
};
#endif
