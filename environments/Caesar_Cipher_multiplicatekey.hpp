#ifndef CAESAR_CIPHER_MULTIPLICATEKEY_H
#define CAESAR_CIPHER_MULTIPLICATEKEY_H

#include"Caesar_Cipher.h"
#include<vector>

class Caesar_Cipher_multiplicatekey : public Caesar_Cipher
{
public:

		Caesar_Cipher_multiplicatekey(Random* random, int const number_of_key);
//		~Caesar_Cipher();
		
		//All Reinforcement Problems have the observartion variable, although it is not declared here!
		//double* observation;		
		//int number_of_observation_vars;
		//int number_of_action_vars;
		//int trial;
		
		//Reinforcement Problem API
		void start(int &number_of_observation_vars, int& number_of_action_vars);
		double step(double* action);
		double restart();
//		void print();
	
		Random* random;

private:
        int const number_of_key;
        double const max_value_of_each_key;
        std::vector<double> keys;
};

#endif
