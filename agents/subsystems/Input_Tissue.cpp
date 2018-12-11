
#include"Input_Tissue.h"

Input_Tissue::Input_Tissue(int number_of_outputs)
{
	this->number_of_outputs = number_of_outputs;

	output= (double*)malloc(sizeof(double)*number_of_outputs);	

	double initial_output_value=-1;

	for(int i=0; i<number_of_outputs; ++i)
	{
		output[i]= initial_output_value;
	}
	
	//set the survival bit to false, the default value
	survived=false;
}

Input_Tissue::~Input_Tissue()
{
	
}

void Input_Tissue::execute(double* observation)
{
	for(int i=0; i<number_of_outputs; ++i)
	{
		this->output[i]= observation[i];
		//printf("%f ",this->input[i]);
	}
	//printf("\n");
}
