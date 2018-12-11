///////////////////////////////////////////////////////////////////
//
//			- SubSystem  - 
//
//		Any SubSystem can be easily pluged in and out of the Core System.
//		Each one provides some specific functions and/or information.
//		For example: past input/output memory, novelty information, abstraction, etc....
//
//		SubSystems have two main ways of working:
//		1. Evolving Tissues - They will create and maintain a set of different Tissues.
//		Every Tissue will be created and connected with a single Individual of the Core System (createNewTissue()).
//		Good individuals will keep the Tissues alive and bad ones will die and remove the Tissues (mutateTissue).
//		Novel individuals will receive new Tissues (return value of mutateTissue), but this time around the Tissues that survived
//		can be used as a basis with some mutation to create yet more promising Tissues, instead of random ones.
//		2. Static Tissue - There is only a single Tissue in place shared by all individuals.
//		The createNewTissue() function will always return the same Tissue and the mutateTissue() function will 
//		do nothing and return the only available Tissue.
//
///////////////////////////////////////////////////////////////////


#ifndef SUBSYSTEM_H	
#define SUBSYSTEM_H

#include"../modules/structural_dna.h"
#include"stdlib.h"
#include<stack>
#include<list>
#include"useful/useful_utils.h"
#include"../../parameters.h"
#include"Tissue.h"

class SubSystem
{
	public:

		//SubSystem();
		//~SubSystem();

		//static void setRandom(Random* random);

		/////////////  API ///////////////////
		//The complete overview of the API is 
		//written above
		//////////////////////////////////////
	
		virtual void execute(double* input)=0;	
		//create new Tissue
		virtual Tissue* createNewTissue()=0;
		//remove a previous created Tissue and create and return a new one. 
		//IMPORTANT: the new created Tissue should be similar to the one removed
		virtual Tissue* mutateTissue(Tissue* remove_tissue)=0;
		
		//remove the unmarked Tissues (see markSurvivedTissue() in Tissue.h)
		virtual void cleanNonMarkedTissues()=0;

};

#endif
