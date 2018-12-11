
#include"Input_SubSystem.h"

Input_SubSystem::Input_SubSystem(int number_of_input_variables)
{
	tissue=	new Input_Tissue(number_of_input_variables); 
}

Input_SubSystem::~Input_SubSystem()
{

}

Tissue* Input_SubSystem::createNewTissue()
{
	//always return the same tissue
	return tissue;
}

Tissue* Input_SubSystem::mutateTissue(Tissue* remove_tissue)
{
	//do nothing
	//there is just one Tissue anyway
	return tissue;
}

void Input_SubSystem::cleanNonMarkedTissues()
{
	//do nothing
}

//update the output of the system
void Input_SubSystem::execute(double* input)
{
	tissue->execute(input);	
}



