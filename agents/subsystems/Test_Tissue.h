#ifndef TEST_TISSUE_H	
#define TEST_TISSUE_H

#include"Tissue.h"

class Test_Tissue : public Tissue
{
	public:

		Test_Tissue(int input_size, int output_size);
		~Test_Tissue();

		/////////////  API ///////////////////

		//double* input;
		//double* output;
		//int input_size;
		//int output_size;
};

#endif
