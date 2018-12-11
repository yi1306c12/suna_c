
#include"XCS_Butz.h"

XCS_Butz::XCS_Butz()
{
	FILE *parameterFile=NULL;

	random= new State_of_Art_Random(time(NULL));
      	if( (parameterFile =fopen("xcs_butz_parameter", "rt")) == NULL)  
	{
		printf("Error: could not load parameter file\n");
		exit(1);
	}

  	xcsDefault = initializeXCS(parameterFile);
  
	setSeed(xcsDefault->initialSeed);
	
	//fprintXCS(stdout, xcsDefault);

}

XCS_Butz::~XCS_Butz()
{
	freeXCS(xcsDefault);
}

void XCS_Butz::init(int number_of_observation_vars, int number_of_action_vars)
{
	this->number_of_observation_vars= number_of_observation_vars;
	this->number_of_action_vars= number_of_action_vars;

	action= (double*) malloc(number_of_action_vars*sizeof(double));
}

void XCS_Butz::step(double* observation, double reward)
{
	
}

void XCS_Butz::print()
{

}
		
void XCS_Butz::endEpisode()
{

}

double XCS_Butz::stepBestAction(double* observation)
{
	return 0;
}

void XCS_Butz::saveAgent(const char* filename)
{
	printf("saveAgent() not implemented\n");

}

void XCS_Butz::loadAgent(const char* filename)
{
	printf("loadAgent() not implemented\n");
}

