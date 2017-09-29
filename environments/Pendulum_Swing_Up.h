
#ifndef PENDULUM_SWING_UP_H
#define PENDULUM_SWING_UP_H

#include"../parameters.h"
#include"Reinforcement_Environment.h"
#include"../agents/Reinforcement_Agent.h"
#include"random/State_of_Art_Random.h"
#include"stdlib.h"
#include"stdio.h"
  	
#define FORCE_MAG 10.0
#define DEGREES_180 3.141593
   
class Pendulum_Swing_Up : public Reinforcement_Environment
{
	public:

		Pendulum_Swing_Up(Random* random);
		~Pendulum_Swing_Up();
		
		//All Reinforcement Problems have the observartion variable, although it is not declared here!
		//double* observation;		
		//int number_of_observation_vars;
		//int number_of_action_vars;
		//int trial;
		
		//Reinforcement Problem API
		void start(int &number_of_observation_vars, int& number_of_action_vars);
		double step(double* action);
		double restart();
		void print();
	
		//auxiliary functions
		void cart_pole(double force, double *x,double *x_dot, double *theta, double *theta_dot);

		Random* random;
		double x, x_dot,theta,theta_dot;
};

#endif
