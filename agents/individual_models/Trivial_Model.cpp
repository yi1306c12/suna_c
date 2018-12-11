
#include"Trivial_Model.h"

Trivial_Model::Trivial_Model(int number_of_action_vars, Random* random)
{
	this->dna_size= number_of_action_vars;
	this->random= random;
	dna= (double*)malloc(sizeof(double)*dna_size);
	this->numerosity= 0;

	int i;
	for(i=0;i<dna_size;++i)
	{
		dna[i]=random->uniform(-1.0, 1.0);
	}
}

Trivial_Model::~Trivial_Model()
{

}

//only copy the new dna
void Trivial_Model::updateModel(double* dna)
{
	memcpy(this->dna, dna, dna_size*sizeof(double));
}

//trivial output, the dna is the output
void Trivial_Model::getOutput(double* input, double* output)
{
	memcpy(output, dna, dna_size*sizeof(double));
}

double Trivial_Model::diversity(Individual_Model* model)
{
	int i;
	double diversity=0;
	Trivial_Model* trivial_model= (Trivial_Model*) model;

	for(i=0;i<dna_size;++i)
	{
		diversity+= (this->dna[i] - trivial_model->dna[i])*
		(this->dna[i] - trivial_model->dna[i]);
	}
	diversity= diversity/dna_size;

	return diversity;
}
