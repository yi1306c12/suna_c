///////////////////////////////////////////////////////////////////
//
//			- Input_SubSystem  - 
//
//			Provides the input. It makes easier to add
//			an abstraction layer later.
//
//
///////////////////////////////////////////////////////////////////


#ifndef INPUT_SUBSYSTEM_H	
#define INPUT_SUBSYSTEM_H

#include"SubSystem.h"
#include"Input_Tissue.h"

class Input_SubSystem : public SubSystem
{
	public:

		Input_SubSystem(int number_of_input_variables);
		~Input_SubSystem();	

		Input_Tissue* tissue;

		//static void setRandom(Random* random);

		//API
		void execute(double* input);
		Tissue* createNewTissue();
		Tissue* mutateTissue(Tissue* remove_tissue);
		
		void cleanNonMarkedTissues();
		
		//void mutation(Tissue* tissue);

};

#endif
