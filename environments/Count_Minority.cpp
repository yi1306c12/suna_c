#include"Count_Minority.h"

#include<stdexcept>
using std::runtime_error;

#include<vector>
#include<bitset>
using std::vector;
using std::bitset;
#include<cmath>

#include<iostream>
using std::cout;
using std::endl;

Count_Minority::Count_Minority(Random* random)
: random(random) 
{
    MAX_STEPS = 100000;
    if (data_length < answer_length) throw runtime_error("data length < answer length");

    //make answer
    //memory allocation
    problems = vector<vector<bitset<data_length> > >(answer_length);
    //make each bit answer
    for (unsigned long n = 1, nend = static_cast<unsigned long>(std::pow(2,data_length)); n < nend; ++n)
    {
        bitset<data_length> num(n);
        int const rise = num.count();
        if(rise <= answer_length)
        {
            problems[rise-1].push_back(num);
            problems[rise-1].push_back(~num);
        }
    }
}


void Count_Minority::start(int &number_of_observation_vars, int& number_of_action_vars)
{
    number_of_observation_vars = data_length;
    number_of_action_vars = answer_length;

    this->number_of_observation_vars = number_of_observation_vars;
    this->number_of_action_vars = number_of_action_vars;

    //allocation
    observation= (double*) malloc(number_of_observation_vars*sizeof(double));

    //Initialize
    restart();
}


#include<iostream>
using std::cout;
using std::endl;
double Count_Minority::step(double* action)
{
    double const one_threshold = 1.;

    if(action==NULL)
    {
        //throw runtime_error("NULL action");
        return 0.;
    }

//Evaluate with -|response - answer| or (response == answer)?
    //Evaluation
    int reward = 0;
    for(int i = 0; i < number_of_action_vars; ++i)
    {
        bool const is_one = action[i] > one_threshold;
        bool const answer_i = i == previous_answer-1;
        reward -= answer_i != is_one;
    }

//whether data&answer change every step?
    //Update Values
    int const answer = random->uniform(0,answer_length-1);
    int const obs_rand = random->uniform(0,problems[answer].size()-1);


    bitset<data_length> const obs = problems[answer][obs_rand];
    for(int i = 0; i < number_of_observation_vars; ++i)
    {
        observation[i] = static_cast<double>(obs[i]);
    }

    previous_answer = answer;
    return static_cast<double>(reward);
}


double Count_Minority::restart()
{
    ++trial;
    previous_answer = 0;
    for(int i=0; i<number_of_observation_vars; ++i)
    {
        observation[i] = 0.;
    }

    return 0.;
}