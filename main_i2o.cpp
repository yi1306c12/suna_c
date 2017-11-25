#include"environments/Double_Cart_Pole.h"
#include"agents/modules/Module.h"

#include<ctime>
int main(const int argc, const char* argv[])
{
    Random random = State_of_Art_Random(time(NULL));

    //init environment
    int number_of_observation, number_of_action;
    Reinforcement_Environment env = Double_Cart_Pole(random);
    env.start(number_of_observation,number_of_action);

    //init network
    Module module(number_of_observation, number_of_action);
    module.addConnection(2,3,-1,100);

    //print network
    

    return 0;
}