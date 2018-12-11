
#include"Neural_Model.h"

Neural_Model::Neural_Model(int number_of_inputs, int number_of_hidden, int number_of_outputs, int number_of_hidden_layers, Random* random)
{
	this->random= random;
	this->numerosity= 0;

	neuron= new Simple_Feedforward(number_of_inputs, number_of_hidden, number_of_outputs, number_of_hidden_layers, random);
	
	this->dna_size= neuron->numberOfWeightsForTheArchitecture();
	dna= (double*)malloc(sizeof(double)*dna_size);
	int i;
	for(i=0;i<dna_size;++i)
	{
		dna[i]=random->uniform(-1.0, 1.0);
	}

	neuron->setWeights(dna);

	//neuron->print();
}

Neural_Model::~Neural_Model()
{

}

//only copy the new dna
void Neural_Model::updateModel(double* dna)
{
	//update the stored dna
	memcpy(this->dna, dna, dna_size*sizeof(double));
	
	//update model
	neuron->setWeights(this->dna);
}

//trivial output, the dna is the output
void Neural_Model::getOutput(double* input, double* output)
{
	neuron->activate(input,output);
}
		
double Neural_Model::diversity(Individual_Model* model)
{
	int i;
	double diversity=0;
	Neural_Model* neural_model= (Neural_Model*) model;

	for(i=0;i<dna_size;++i)
	{
		diversity+= (this->dna[i] - neural_model->dna[i])*
		(this->dna[i] - neural_model->dna[i]);
	}
	diversity= diversity/dna_size;

	return diversity;
}
		
//Simple_Feedforward* Neural_Model::neuron= NULL;
