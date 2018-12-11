#ifndef INPUT_TISSUE_H	
#define INPUT_TISSUE_H

#include"Tissue.h"

class Input_Tissue : public Tissue
{
	public:

		Input_Tissue(int input_size);
		~Input_Tissue();

		/////////////  API ///////////////////

		//double* input;
		//double* output;
		//int number_of_inputs;
		//int number_of_outputs;
		
		//bool survived;
		
		//API
		void execute(double* observation);
};

#endif
