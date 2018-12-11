
#include"stdio.h"
#include"stdlib.h"
#include"Trivial_Model.h"
#include"Neural_Model.h"
#include"time.h"
#include"random/State_of_Art_Random.h"
#include"useful/useful_utils.h"

int main()
{

	Random* random= new State_of_Art_Random(time(NULL));
	int n_output=2;
	int n_input=2;
	//Individual_Model* a= new Trivial_Model(n_output, random);
	int hidden_nodes=2;
	Individual_Model* a= new Neural_Model(n_input, hidden_nodes, n_output, 1, random);

	double input[2];
	double output[2];

	printArray(a->dna,2);
	a->getOutput(input, output);
	printArray(output,2);

	double d[]={1.0,1.0};

	a->updateModel(d);
	printArray(a->dna,2);
	a->getOutput(input, output);
	printArray(output,2);


	
	return 0;
}
