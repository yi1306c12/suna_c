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



/*
    //make answer
    //memory allocation
    //make each bit answer
    problems = vector<bitset<data_length> >(answer_length);

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
    */

    for(int i = 1;i <= answer_length; ++i)
    {
        bitset<data_length> n(0);
        for(int j = 0; j < i;++j) n.set(j);//000..00111

        for(int j = 0; j < data_length-i+1;++j)
        {
            problems.push_back(n);
            problems.push_back(~n);
            n <<= 1;
        }
    }
    for (auto&& v:problems)cout << v << endl;
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
    int const obs_rand = random->uniform(0,problems.size()-1);
    bitset<data_length> const obs = problems[obs_rand];
    for(int i = 0; i < number_of_observation_vars; ++i)
    {
        observation[i] = static_cast<double>(obs[i]);
    }
    previous_answer = obs.count();
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