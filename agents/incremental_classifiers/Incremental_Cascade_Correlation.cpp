
#include"Incremental_Cascade_Correlation.h"

Incremental_Cascade_Correlation::Incremental_Cascade_Correlation(int number_of_inputs, int number_of_outputs, Random* random)
{
	
	enum fann_train_enum training_algorithm = FANN_TRAIN_RPROP;
	//enum fann_train_enum training_algorithm = FANN_TRAIN_QUICKPROP;
	bool multi= false;
	fann_type steepness;
	enum fann_activationfunc_enum activation;
	this->random= random;
	
	ann = fann_create_shortcut(2, number_of_inputs, number_of_outputs);
		
	fann_set_training_algorithm(ann, training_algorithm);
	fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
	fann_set_activation_function_output(ann, FANN_LINEAR);
	fann_set_train_error_function(ann, FANN_ERRORFUNC_LINEAR);

	this->number_of_inputs= number_of_inputs;
	this->number_of_outputs= number_of_outputs;

	window_size=100000;
	data_counter=0;

	//allocation
	data= alloc(window_size, number_of_inputs, number_of_outputs);
	output= (double*)malloc(sizeof(double)*number_of_outputs);
	fann_output = (fann_type *) malloc(number_of_outputs*sizeof(fann_type));
	fann_input = (fann_type *) malloc(number_of_inputs*sizeof(fann_type));
	
	if(!multi)
	{
		/*steepness = 0.5;*/
		steepness = 0.001;
		fann_set_cascade_activation_steepnesses(ann, &steepness, 1);
		/*activation = FANN_SIN_SYMMETRIC;*/
		activation = FANN_SIGMOID_SYMMETRIC;
		
		fann_set_cascade_activation_functions(ann, &activation, 1);		
		fann_set_cascade_num_candidate_groups(ann, 8);
	}	
		
	if(training_algorithm == FANN_TRAIN_QUICKPROP)
	{
		fann_set_learning_rate(ann, 0.35);
		fann_randomize_weights(ann, -2.0,2.0);
	}
	
	fann_set_bit_fail_limit(ann, 0.9);
	//fann_set_train_stop_function(ann, FANN_STOPFUNC_BIT);
	fann_set_train_stop_function(ann, FANN_STOPFUNC_MSE);
	//fann_print_parameters(ann);
		
	fann_save(ann, "cascade_train2.net");

}

Incremental_Cascade_Correlation::~Incremental_Cascade_Correlation()
{
	printf("Saving network.\n");
	
	fann_save(ann, "cascade_train.net");
	
	printf("Cleaning up.\n");
	fann_destroy(ann);
	
}


void Incremental_Cascade_Correlation::incrementSamples(double *input, double *output) 
{
	unsigned int i,j;
	
	i= data_counter;

	if(data_counter == window_size-1)
	{
		return;
	}

	for(j = 0; j < data->num_input; j++) 
	{
		data->input[i][j] = input[j];
	}

	for(j = 0; j != data->num_output; j++) 
	{
		data->output[i][j] = output[j];
	}
	
	//updating the counter and the size of the data
	if(data->num_data < window_size)
	{
		data->num_data++;
		data_counter++;
		if(data_counter >= window_size)
		{
			data_counter= 0;
		}
	}
	else
	{
		data_counter++;
		//if the counter points beyond the last sample, reset it
		if(data_counter >= window_size)
		{
			data_counter= 0;
		}
	}
	

	///////////// Train Network On the Updated Dataset //////////////////
	const float desired_error = (const float)0.01;
	unsigned int max_neurons = 10;
	unsigned int neurons_between_reports = 1;
	
	//if(random->uniform(0.0,1.0) > 0.99999)
	if(data_counter == window_size -1 )
	{
		fann_cascadetrain_on_data(ann, data, max_neurons, neurons_between_reports, desired_error);
	}

}

double* Incremental_Cascade_Correlation::predict(double* input)
{
	int i;
	for(i=0;i<number_of_inputs;++i)
	{
		fann_input[i]= input[i];
	}

	fann_output= fann_run(ann, fann_input);
	
	for(i=0;i<number_of_outputs;++i)
	{
		output[i]= fann_output[i];
	}

	return output;
}


struct fann_train_data *Incremental_Cascade_Correlation::alloc(unsigned int samples, unsigned int num_input, unsigned int num_output) 
{
	unsigned int i;
	fann_type *data_input, *data_output;
	struct fann_train_data *data = 	(struct fann_train_data *) malloc(sizeof(struct fann_train_data));
	if(data == NULL) 
	{
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		return NULL;
	}

	fann_init_error_data((struct fann_error *) data);

	data->num_data = 0;
	data->num_input = num_input;
	data->num_output = num_output;
	data->input = (fann_type **) calloc(samples, sizeof(fann_type *));
	if(data->input == NULL) 
	{
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(data);
		return NULL;
	}

	data->output = (fann_type **) calloc(samples, sizeof(fann_type *));
	if(data->output == NULL) 
	{
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(data);
		return NULL;
	}

	data_input = (fann_type *) calloc(num_input * samples, sizeof(fann_type));
	if(data_input == NULL) 
	{
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(data);
		return NULL;
	}

	data_output = (fann_type *) calloc(num_output * samples, sizeof(fann_type));
	if(data_output == NULL) 
	{
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(data);
		
		return NULL;
	}


	for(i = 0; i != samples; i++) 
	{
		data->input[i] = data_input;
		data_input += num_input;

		data->output[i] = data_output;
		data_output += num_output;

	}
	return data;
}
