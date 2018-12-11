///////////////////////////////////////////////////////////////////
//
//			- Test_SubSystem  - 
//
//			The first SubSystem. It was created for testing purposes.
//
///////////////////////////////////////////////////////////////////


#ifndef TEST_SUBSYSTEM_H	
#define TEST_SUBSYSTEM_H

#include"SubSystem.h"
#include"Test_Tissue.h"

class Test_SubSystem : public SubSystem
{
	public:

		Test_SubSystem();
		~Test_SubSystem();	

		Test_Tissue* tissue;

		//static void setRandom(Random* random);

		//API
		Tissue* createNewTissue();
		void removeTissue(Tissue* tissue);
		
		
		void mutation(Tissue* tissue);

};

#endif
