#include"Simple_Feedforward.h"

Simple_Feedforward::Simple_Feedforward(int number_of_inputs, int number_of_hidden, int number_of_outputs, int number_of_hidden_layers, Random* random)
{
	this->random= random;

	this->number_of_inputs= number_of_inputs;
	this->number_of_hidden= number_of_hidden;
	this->number_of_outputs= number_of_outputs;
	this->number_of_hidden_layers= number_of_hidden_layers;

	allocateWithRandomWeights();
	//allocateWithoutSettingWeights();

}

Simple_Feedforward::~Simple_Feedforward()
{

}

void Simple_Feedforward::allocateWithoutSettingWeights()
{
	int i,j,k;
	
	//create input nodes
	input= new Graph_Node *[number_of_inputs];
	for(i=0;i<number_of_inputs;++i)
	{
		input[i]= new Graph_Node();
	}
	
	//allocate bias
	bias= (double**) malloc(sizeof(double*)*number_of_hidden_layers);
	alpha= (double**) malloc(sizeof(double*)*number_of_hidden_layers);
	bias_output = (double*) malloc(sizeof(double)*number_of_outputs);

	//create hidden nodes
	hidden= new Graph_Node** [number_of_hidden_layers];
	for(i=0;i<number_of_hidden_layers;++i)
	{
		hidden[i]= new Graph_Node *[number_of_hidden];
		bias[i]= (double*) malloc(sizeof(double)*number_of_hidden);
		alpha[i]= (double*) malloc(sizeof(double)*number_of_hidden);
	}
	
	
	for(i=0;i<number_of_hidden;++i)
	{
		hidden[0][i]= new Graph_Node();
		
		for(j=0;j<number_of_inputs;++j)
		{
			double random_weight= 0;
			input[j]->insertConnection(hidden[0][i], random_weight);
			//input[j]->insertConnection(hidden[i], 1);
		}

		bias[0][i]= 0;
		alpha[0][i]= 0;
		//alpha[0][i]=1;

	}
	
	for(j=1;j<number_of_hidden_layers;++j)
	{
		for(i=0;i<number_of_hidden;++i)
		{
			hidden[j][i]= new Graph_Node();
			
			//connect all hidden from previous layer to this layer
			for(k=0;k<number_of_hidden;++k)
			{
				double random_weight= 0;
				hidden[j-1][k]->insertConnection(hidden[j][i], random_weight);
				//input[j]->insertConnection(hidden[i], 1);
			}

			bias[j][i]= 0;
			alpha[0][i]= 0;
			//alpha[j][i]=1;
		}
	}
	
	//create output nodes
	output= new Graph_Node *[number_of_outputs];
	for(i=0;i<number_of_outputs;++i)
	{
		output[i]= new Graph_Node();
		
		for(j=0;j<number_of_hidden;++j)
		{
			double random_weight= 0;
			hidden[number_of_hidden_layers-1][j]->insertConnection(output[i], random_weight);
			//hidden[j]->insertConnection(output[i], 1);
		}
		
		bias_output[i]= 0;
	}
}

void Simple_Feedforward::allocateWithRandomWeights()
{
	int i,j,k;
	
	//create input nodes
	input= new Graph_Node *[number_of_inputs];
	for(i=0;i<number_of_inputs;++i)
	{
		input[i]= new Graph_Node();
	}
	
	//allocate bias
	bias= (double**) malloc(sizeof(double*)*number_of_hidden_layers);
	alpha= (double**) malloc(sizeof(double*)*number_of_hidden_layers);
	bias_output = (double*) malloc(sizeof(double)*number_of_outputs);

	//create hidden nodes
	hidden= new Graph_Node** [number_of_hidden_layers];
	for(i=0;i<number_of_hidden_layers;++i)
	{
		hidden[i]= new Graph_Node *[number_of_hidden];
		bias[i]= (double*) malloc(sizeof(double)*number_of_hidden);
		alpha[i]= (double*) malloc(sizeof(double)*number_of_hidden);
	}
	
	for(i=0;i<number_of_hidden;++i)
	{
		hidden[0][i]= new Graph_Node();
		
		for(j=0;j<number_of_inputs;++j)
		{
			double random_weight= random->uniform(-1.0,1.0);
			input[j]->insertConnection(hidden[0][i], random_weight);
			//input[j]->insertConnection(hidden[i], 1);
		}

		bias[0][i]= random->uniform(-1.0,1.0);
		alpha[0][i]= random->uniform(-1.0,1.0);
		//alpha[0][i]=1;

	}
	
	for(j=1;j<number_of_hidden_layers;++j)
	{
		for(i=0;i<number_of_hidden;++i)
		{
			hidden[j][i]= new Graph_Node();
			
			//connect all hidden from previous layer to this layer
			for(k=0;k<number_of_hidden;++k)
			{
				double random_weight= random->uniform(-1.0,1.0);
				hidden[j-1][k]->insertConnection(hidden[j][i], random_weight);
				//input[j]->insertConnection(hidden[i], 1);
			}

			bias[j][i]= random->uniform(-1.0,1.0);
			alpha[j][i]= random->uniform(-1.0,1.0);
			//alpha[j][i]=1;
		}
	}
	
	//create output nodes
	output= new Graph_Node *[number_of_outputs];
	for(i=0;i<number_of_outputs;++i)
	{
		output[i]= new Graph_Node();
		
		for(j=0;j<number_of_hidden;++j)
		{
			double random_weight= random->uniform(-1.0,1.0);
			hidden[number_of_hidden_layers-1][j]->insertConnection(output[i], random_weight);
			//hidden[j]->insertConnection(output[i], 1);
		}
			
		bias_output[i]= random->uniform(-1.0,1.0);
	}
}

void Simple_Feedforward::activate(double* stimulus, double* response) 
{
	int i,j;
	
	
	//transfering input's information to hidden nodes
	for(i=0;i<number_of_inputs;++i)
	{
		input[i]->value= stimulus[i];
		
		//input[i]->value= tanh(input[i]->value);
		//input[i]->value= 1/(1+exp(-input[i]->value));
		//input[i]->value= exp(-input[i]->value);
		
		//RBF functions
		//input[i]->value= exp(-((input[i]->value)*(input[i]->value)));
		//input[i]->value= sqrt(1+(epsilon*input[i]->value)*(epsilon*input[i]->value));
		//input[i]->value= input[i]->value*input[i]->value*log(input[i]->value);
	
		input[i]->passValue();
	}
	
	//processing hidden nodes
	for(j=0;j<number_of_hidden_layers;++j)
	{
		for(i=0;i<number_of_hidden;++i)
		{
			//sum the bias, variable not affected by any connection weight
			hidden[j][i]->value+= bias[j][i]; 
		
			//double alpha= 1.0;

		//	hidden[j][i]->value= 1/(1+exp(-alpha[j][i]*hidden[j][i]->value));
			hidden[j][i]->value= tanh(hidden[j][i]->value);
			//hidden[j][i]->value= 1.0/(1.0+exp(-5.0*hidden[j][i]->value));
		//	hidden[j][i]->value= exp(-alpha[j][i]*hidden[j][i]->value);

			hidden[j][i]->passValue();
		}
	}
	
	//clean output nodes and set output_vector
	for(i=0;i<number_of_outputs;++i)
	{
		output[i]->value+= bias_output[i];

		response[i]= output[i]->value;
		//response[i]= tanh(output[i]->value);
		//response[i]= 1.0/(1.0+exp(-5.0*output[i]->value));

		//by passing the value to the connected nodes, after it, the value become 0
		output[i]->passValue();
	}

	//Graph_Node::printValue(input,number_of_inputs,"last.dot");

}

//Set the weights conforming to the dna
//
//the dna is composed of 2 matrices and one line between them with the bias values
//one matrix is a map of weights between the "input nodes x hidden nodes"
//one line after that is the bias of the hidden nodes
//the other matrix is the weights of "hidden nodes x output nodes"
//
//For example if the input, hidden and output nodes are 2, 3, 1, and 2 hidden layerswe would have:
//
//input x hidden
//[ x x x ] 
//[ x x x ]
//
//bias
//[ x x x ]
//[ x x x ]
//
//hidden x hidden
//
//[ x x x ]
//[ x x x ]
//[ x x x ]
//
//hidden x output
//[ x ]
//[ x ]
//[ x ]
//
//bias for the output (output)
//[ x ]
//[ x ]
//[ x ]
//
void Simple_Feedforward::setWeights(double* dna)
{
	int i,j,k,l,e,b;

	for(i=0; i<number_of_inputs; ++i)
	{
		memcpy(input[i]->connection_weight, (dna + i*number_of_hidden), sizeof(double)*number_of_hidden);
	}
		
	//copy bias
	for(k=0; k<number_of_hidden_layers; ++k)
	{
		memcpy(bias[k],(dna + i*number_of_hidden + k*number_of_hidden), sizeof(double)*number_of_hidden);
	}
	

	//i++

	//hidden x hidden
	j=0;
	for(l=0; l<number_of_hidden_layers-1; ++l)
	{
		for(j=0; j<number_of_hidden; ++j)
		{
			memcpy(hidden[l][j]->connection_weight, (dna + (i*number_of_hidden + k*number_of_hidden + l*number_of_hidden*number_of_hidden + j*number_of_hidden)), sizeof(double)*number_of_hidden);
		}
	}

	//hidden x output
	for(e=0; e<number_of_hidden; ++e)
	{
		memcpy(hidden[number_of_hidden_layers-1][e]->connection_weight, (dna + (i*number_of_hidden + k*number_of_hidden + j*number_of_hidden*l + e*number_of_outputs)), sizeof(double)*number_of_outputs);
	}
	
	//copy bias for the output
	for(b=0; b<number_of_hidden_layers; ++b)
	{
		memcpy(bias_output, (dna + (i*number_of_hidden + k*number_of_hidden + j*number_of_hidden*l + e*number_of_outputs)), sizeof(double)*number_of_outputs);
	}
}

void Simple_Feedforward::print()
{
	Graph_Node::printValue(input,number_of_inputs,"simple_feedfoward.dot");

	printf("Output Bias\n");
	printArray(bias_output,number_of_outputs);
}

//calculate the number of weights present in the architecture
int Simple_Feedforward::numberOfWeightsForTheArchitecture()
{
	int layers= number_of_hidden_layers;

	int number_of_weights= (number_of_inputs+number_of_outputs)*number_of_hidden+number_of_hidden*(layers-1)*number_of_hidden + number_of_hidden*layers + number_of_outputs;

	return number_of_weights;
}
/*
double Simple_Feedforward::activateNeuron(double* input, int input_lenght)
{
        int i;

        double sum=0;
        for(i=0;i<input_lenght;++i)
        {
                sum+=input[i];
        }

        switch(activation_function_type)
        {
                case HYPERBOLIC:
                {
                        return tanh(sum);
                }
        }

}*/

