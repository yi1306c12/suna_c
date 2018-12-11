
#include"Test_Tissue.h"

Test_Tissue::Test_Tissue(int input_size, int output_size)
{
	this->input_size= input_size;
	this->output_size= output_size;

	input= (double*)malloc(sizeof(double)*input_size);	
	output= (double*)malloc(sizeof(double)*output_size);	

	double initial_input_value=-1;
	double initial_output_value=-1;

	for(int i=0; i<input_size; ++i)
	{
		input[i]= initial_input_value;
	}
	
	for(int i=0; i<output_size; ++i)
	{
		output[i]= initial_output_value;
	}
}

Test_Tissue::~Test_Tissue()
{
	
}
