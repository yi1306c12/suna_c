
#include"Go.h"


Go::Go(Random* random, int boardsize)
{
	trial=-1;
	steps=0;
	this->random= random;

	this->boardsize= boardsize;

	//actually this is just a very big number, because they should never stop playing in the middle of the game
	this->MAX_STEPS= MAX_GO_STEPS;

	/******************** Spawning a gnugo process *************************/

        pipe(read_fd);
        pipe(write_fd);

        //spawn a child 
        pid_t pid=fork();

        //child process
        if(pid==0)
        {
		//substituting the stdin and stdout for a pipe
		dup2(CHILD_READ, STDIN_FILENO);
		dup2(CHILD_WRITE, STDOUT_FILENO);

		close((read_fd[0]));
		close((read_fd[1]));
		close((write_fd[0]));
		close((write_fd[1]));

		execl("/usr/bin/gnugo","/usr/bin/gnugo","--mode","gtp",NULL);
        }
        // pid!=0; parent process
        else
        {
		//closes output
		close(CHILD_WRITE);
		//closes input
		close(CHILD_READ);

		//define the boardsize
		sprintf(message,"boardsize %d\n",boardsize);
		write(PARENT_WRITE, message, (strlen(message)));

		//read confirmation message
		readMessage(received_message);

	}
	
	/*******************************************************/
}

Go::~Go()
{

}

void Go::start(int &number_of_observation_vars, int& number_of_action_vars)
{
	number_of_observation_vars= 2;//boardsize*boardsize;
	this->number_of_observation_vars= 2;//boardsize*boardsize;
	observation= (double*) malloc(number_of_observation_vars*sizeof(double));

	number_of_action_vars=2;//boardsize*boardsize;
	this->number_of_action_vars=2;

	restart();
}

double Go::step(double* action)
{
	steps++;

	// initial reward
	if(action==NULL)
	{
		return 0;
	}

	int move[2];
	move[0]=(int)action[0];
	move[1]=(int)action[1];

	//send move to GnuGo	
	moveToMessage(move);		
	//read confirmation message
	readMessage(received_message);

	//let GnuGo make its move
	write(PARENT_WRITE, "genmove white\n", 14);
	//printf("sent message genmove white\n");
	readMessage(received_message);
	

	//if GnuGo passes, finish the game
	if(!strncmp("= PASS",received_message,6))
	{
		//let GnuGo make its move
		write(PARENT_WRITE, "final_score\n", 12);
		//printf("sent message final score\n");
		readMessage(received_message);
		
		//get final score
		double reward= messageToReward(received_message);

		restart();

		return reward;
	}
	else
	{
		messageToMove(move, received_message);
	
		//let GnuGo's last move be observed
		observation[0]=move[0];		
		observation[1]=move[1];		
	}

	//evaluate the current state
	write(PARENT_WRITE, "estimate_score\n", 15);
	//printf("sent message estimate_score\n");
	readMessage(received_message);
	
	double score= messageToReward(received_message);	

	return score;
}

double Go::restart()
{
	trial++;
	steps=0;

	//send restart command to gnugo
	sprintf(message,"clear_board\n");
	write(PARENT_WRITE, message, (strlen(message)));
		
	observation[0]=0;		
	observation[1]=0;		

	//read confirmation message
	readMessage(received_message);

	return 0;
}

void Go::print()
{
	//print board
	sprintf(message,"showboard\n");
	write(PARENT_WRITE, message, (strlen(message)));
	
	readMessage(received_message);
}

void Go::readMessage(char* received_msg)
{
	//printf("reading message\n");
	char buffer[100];
	int count = read(PARENT_READ, buffer, sizeof(buffer)-1);
	buffer[count]=0;   

	//clean the received_msg
	received_msg[0]=0;
	
	sprintf(received_msg,"%s%s",received_msg,buffer);
	while(buffer[count-1]!=10 || buffer[count-2]!=10)
	{
		count = read(PARENT_READ, buffer, sizeof(buffer)-1);
	
		buffer[count]=0;   
		sprintf(received_msg,"%s%s",received_msg,buffer);
	}
	//printf("%s", received_msg);
}

void Go::moveToMessage(int* move)
{
	//printf("%d %d\n",move[0],move[1]);
	if(move[0]>=9)
	{
		move[0]++;	
	}

	if(move[0]<0 || move[0] > 100)
	{
		move[0]=-1;
	}

	char number= move[0]+64;
	sprintf(message,"play black %c%d\n",number,move[1]);
	write(PARENT_WRITE, message, (strlen(message)));
	//printf("sent message: %s\n",message);
	
	//printf("sent\n%s\n",received_msg);

}

void Go::messageToMove(int* move, char* received_msg)
{
	char number= received_msg[2];
	//the char 'A' is the same as 64 in integer
	move[0]= number -64;
	
	//correct the very strange absence of 'I' in the Go board
	if(move[0]>=10)
	{
		move[0]--;
	}

	char buffer[3];
	int i=3;
	for(i=3;received_msg[i]!=10;++i)
	{
		buffer[i-3]=received_msg[i];
	}
	buffer[i-3]=0;
	move[1]=atoi(buffer);

	//printf("read %d %d\n",move[0],move[1]);

}

double Go::messageToReward(char* received_msg)
{
	bool win;
	if(received_msg[2]=='B')
	{
		win=true;
	}
	else
	{
		if(received_msg[2]=='W')
		{
			win=false;
		}
		//draw
		else
		{
			return 0.0;
		}
		
	}
	
	
	char buffer[8];
	int i=3;
	for(i=3;received_msg[i]!=32&&received_msg[i]!=10;++i)
	{
		buffer[i-3]=received_msg[i];
	}
	buffer[i-3]=0;
	double score= atof(buffer);

	if(win==false)
	{
		score= -score;
	}

	score+= (double)steps*10.0;

	//printf("converted score: %f\n",score);
	return score;
}
