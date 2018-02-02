#ifndef COUNTMINORITY
#define COUNTMINORITY



#include"../parameters.h"
#include"Reinforcement_Environment.h"
#include"random/State_of_Art_Random.h"
#include"../agents/Reinforcement_Agent.h"


#include<vector>
#include<bitset>

int const data_length = 10;
int const answer_length = 3;

class Count_Minority : public Reinforcement_Environment
{
    int next_answer;

    std::vector<std::bitset<data_length> > problems;

    int next_bit;
    std::bitset<data_length> now_series;

public:
    Count_Minority(Random* random);
    ~Count_Minority(){};

    Random* const random;

    void start(int &number_of_observation_vars, int& number_of_action_vars);
    double step(double* action);
    double restart();
    void print(){};
    void set(){};

};

#endif