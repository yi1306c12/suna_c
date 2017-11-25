#include"environments/Double_Cart_Pole.h"
#include"agents/modules/Module.h"

#include<ctime>
#include<iostream>
#include<cstdlib>

double test(const int from_id, const int weight, Reinforcement_Environment* env)
{
    //init environment
    int number_of_observation, number_of_action;
    env->start(number_of_observation,number_of_action);

    //init network
    Module module(number_of_observation, number_of_action,100);
    module.addConnection(from_id,3,-1,weight);

    //print network
    //module.printGraph("test_individual.dot");


    double* action = (double*) calloc(number_of_action, sizeof(double));
    double reward = env->step(NULL);
    for(int i = 0;i < env->MAX_STEPS;++i)
    {
        module.process(env->observation, action);
        reward += env->step(action);
    }

    return reward + env->restart();
}

int main(const int argc, const char* argv[])
{
    Random* random = new State_of_Art_Random(time(NULL));
    Reinforcement_Environment* env = new Double_Cart_Pole(random);
    
    for(int i = 0; i < 3; ++i)
    {
        std::cout << "input:" << i << std::endl;
        for(int w = -1000; w < 1000; w+=1)
        {
            std::cout << w << "," << test(i,w,env) << std::endl;
        }
    }
    
    return 0;
}