
/********************************************************************************
 *
 * 	This environment let an algorithm play Go against GnuGo
 *
 * 	GTP 2.0 protocol is used to communicate with GnuGo
 * 	The communication is made between processes using pipes, therefore an Unix/Linux
 * 	based system is required
 *
 *
 * *****************************************************************************/

#ifndef GO_H
#define GO_H

#include"../parameters.h"
#include"Reinforcement_Environment.h"
#include"../agents/Reinforcement_Agent.h"
#include"random/State_of_Art_Random.h"
#include"stdlib.h"
#include"stdio.h"
#include <unistd.h> /* for fork */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h> /* for wait */
#include<string.h>


#define PARENT_READ (read_fd[0])
#define PARENT_WRITE (write_fd[1])
#define CHILD_READ (write_fd[0])
#define CHILD_WRITE (read_fd[1])


class Go : public Reinforcement_Environment
{

	public:
		Go(Random* random, int boardsize);
		~Go();
		
		//API's variables
		//double* observation;		
		//int number_of_observation_vars;
		//int number_of_action_vars;
		//int trial;

		//variables:
		int boardsize;
		Random* random;
        	int steps;

		//pipes to communicate between this and gnugo's process
		int read_fd[2];
        	int write_fd[2];
		char message[64];
		char received_message[1024];
		char buffer[1024];

		//auxiliary functions
		void messageToMove(int* move, char* received_msg);
		void moveToMessage(int* move);
		void readMessage(char* received_msg);
		double messageToReward(char* received_msg);

		//API's functions
		void start(int &number_of_observation_vars, int& number_of_action_vars);
		double step(double* action);
		double restart();
		void print();
};

#endif
