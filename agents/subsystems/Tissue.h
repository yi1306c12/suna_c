///////////////////////////////////////////////////////////////////
//
//			- Tissue  - 
//
//			Tissues are the individuals of a SubSystem.
//			They are blocks of logic doing some specific function.
//
///////////////////////////////////////////////////////////////////


#ifndef TISSUE_H	
#define TISSUE_H

#include"stdlib.h"
#include"stdio.h"

class Tissue
{
	public:

		//Tissue();
		//~Tissue();

		/////////////  API ///////////////////

		double* input;
		double* output;
		int number_of_inputs;
		int number_of_outputs;

		bool survived;

		void markSurvivedTissue(){survived= true;};

		//the input received here 
		virtual void execute(double* original_input)=0;
};

#endif
