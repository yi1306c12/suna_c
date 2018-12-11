#ifndef CACLA
#define CACLA

# include "Cacla.h"


Cacla::Cacla(Random* random) 
{
	printf("Cacla running here\n");
	this->random= random;
	
	endOfEpisode= false;
}

Cacla::~Cacla() 
{
	delete ANN ;
	delete VNN ;

	delete [] VTarget ;
}
	
void Cacla::init(int number_of_observation_vars, int number_of_action_vars)
{
	actionDimension= number_of_action_vars;
	stateDimension= number_of_observation_vars;
	this->number_of_observation_vars= number_of_observation_vars;
	this->number_of_action_vars= number_of_action_vars;
	
	action= (double*) malloc(number_of_action_vars*sizeof(double));
	previous_observation= (double*) malloc(number_of_observation_vars*sizeof(double));
	
	continuousStates=true;
	discreteStates=false;
	continuousActions=true;
	discreteActions=false;

	srand48( clock() ) ;

	nHiddenQ= NUMBER_OF_HIDDEN_NODES_Q;
	nHiddenV= NUMBER_OF_HIDDEN_NODES_V;
	epsilon= EGREEDY_EPSILON;
	gamma= CACLA_GAMMA;
	learningRate[0]= LEARNING_RATE_A;
	learningRate[1]= LEARNING_RATE_V;

	int layerSizesA[] = { stateDimension, nHiddenQ, actionDimension } ;
	int layerSizesV[] = { stateDimension, nHiddenV, 1 } ;

	ANN = new cNeuralNetwork( 1, layerSizesA ) ;
	VNN = new cNeuralNetwork( 1, layerSizesV ) ;

	VTarget = new double[ 1 ] ;

	storedGauss = false ;
}
	
void Cacla::step(double* observation, double reward)
{
	//printf("epis %d\n",endOfEpisode);

	//use the previous observation, the previous action and the current observation to update the model
	//NOTICE: the action passed is equal to the previous action, because it was not updated yet to reflect the current observation
	update(previous_observation, action, reward, observation, endOfEpisode);

	//update the current action to reflect the current observation
	explore(observation, action);

	//store previous observation
	for(int i=0;i<number_of_observation_vars;++i)
	{
		previous_observation[i]= observation[i];
	}

	endOfEpisode= false;

}

void Cacla::endEpisode()
{
	endOfEpisode= true;
}
	
double Cacla::stepBestAction(double* observation)
{
	getMaxAction( observation, action);
	
	//store previous observation
	for(int i=0;i<number_of_observation_vars;++i)
	{
		previous_observation[i]= observation[i];
	}

	//sorry, later I will add the return of the state value (similar to "fitness"), ignored for now
	return 0;
}

void Cacla::print()
{

}

void Cacla::getMaxAction( double * state, double * action ) 
{

	double * As ;

	As = ANN->forwardPropagate( state );

	for( int a = 0 ; a < actionDimension ; a++ ) 
	{
		action[a] = As[a]  ;
	}

}

void Cacla::getRandomAction( double * state, double * action ) 
{

	for( int a = 0 ; a < actionDimension ; a++ ) 
	{
		action[a] = 2.0*drand48() - 1.0;
		//action[a] = 2.0*random->uniform(0.0,1.0) - 1.0;
	}
}


void Cacla::explore( double * state, double * action) 
{

    switch(CACLA_EXPLORATION_TYPE)
    {
	case EGREEDY_EXPLORATION:
		{
        		egreedy( state, action, EGREEDY_EPSILON) ;
			//printf("greedy\n");
		}
	break;

	case GAUSSIAN_EXPLORATION:
		{
			gaussian( state, action, GAUSSIAN_SIGMA) ;
			//printf("gaussian\n");
		}
	break;
    }
}

double Cacla::gaussianRandom() 
{
    // Generates gaussian (or normal) random numbers, with mean = 0 and
    // std dev = 1. Used for gaussian exploration.

    if ( storedGauss ) 
    {

        storedGauss = false ;

        return g2 ;

    } 
    else 
    {

        double x, y ;

        double z = 1.0 ;

        while ( z >= 1.0 ) 
	{
            x = 2.0*drand48() - 1.0;
	    //x = 2.0*random->uniform(0.0,1.0) - 1.0;
            y = 2.0*drand48() - 1.0;
	    //y = 2.0*random->uniform(0.0,1.0) - 1.0;
            z = x * x + y * y;
        }

        z = sqrt( -2.0 * log( z ) / z );

        g1 = x * z;
        g2 = y * z;

        storedGauss = true ;

        return g1 ;

    }

}

void Cacla::gaussian( double * state, double * action, double sigma ) 
{
   
/*	printf("actions\n");

	for ( int a = 0 ; a < actionDimension ; a++ ) 
    	{
		printf("%f, ", action[a]);
	}
	printf("\n");
*/
	
    getMaxAction( state, action ) ;
	
 /*   for ( int a = 0 ; a < actionDimension ; a++ ) 
    	{
		printf("%f, ", action[a]);
	}
	printf("\n");
*/
    for ( int a = 0 ; a < actionDimension ; a++ ) 
    {

        action[a] += sigma*gaussianRandom() ;
	//printf("%f\n",sigma*gaussianRandom());
	//printf("%f\n",action[a]);

    }

}

void Cacla::update( double * state, double * action, double reward, double * nextState, bool endOfEpisode) 
{
	if ( endOfEpisode ) 
	{
		VTarget[ 0 ] = reward ;
	} 
	else 
	{
		double Vs_   = VNN->forwardPropagate( nextState )[0] ;

		VTarget[ 0 ] = reward + gamma*Vs_ ;
	}

	double Vt = VNN->forwardPropagate( state )[0] ;

	VNN->backPropagate( state, VTarget, learningRate[1] ) ;

//	printf("vt0 %f vt %f\n",VTarget[0],Vt);

	if ( VTarget[0] > Vt ) 
	{
		ANN->backPropagate( state, action, learningRate[0] ) ;
	}
}



#endif //CACLA
