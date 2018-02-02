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

//#define ACTION_BUTTON
//#define TIME_SERIES_INPUT

Count_Minority::Count_Minority(Random* random)
: random(random) 
{
    MAX_STEPS = 1000;
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
//    for (auto&& v:problems)cout << v << endl;
}


void Count_Minority::start(int &number_of_observation_vars, int& number_of_action_vars)
{
    number_of_observation_vars = data_length;

#ifdef TIME_SERIES_INPUT
    number_of_observation_vars = 1;
#else
    number_of_observation_vars = data_length;
#endif

#ifdef ACTION_BUTTON
    number_of_action_vars = answer_length+1;
#else
    number_of_action_vars = answer_length;
#endif

    this->number_of_observation_vars = number_of_observation_vars;
    this->number_of_action_vars = number_of_action_vars;
    //allocation
    observation= (double*) malloc(number_of_observation_vars*sizeof(double));

    //Initialize
    restart();
}



double Count_Minority::step(double* action)
{
    if(action==NULL)
    {
        //throw runtime_error("NULL action");
        return 0.;
    }

//Evaluate with -|response - answer| or (response == answer)?
    //Evaluation
    int reward = 0;
#ifdef ACTION_BUTTON
    bool const actioned = action[answer_length <= 0];
    if (actioned)
    {
#endif
        int max_answer = 0;
        double max = action[0];
        for(int i = 1; i < answer_length;++i)
        {
            if(max < action[i])
            {
                max_answer = i;
                max = action[i];
            }
        }
        reward = max_answer == next_answer;
#ifdef ACTION_BUTTON
    }
#endif
//whether data&answer change every step?
    //Update Values

#ifdef TIME_SERIES_INPUT
    if(actioned)
    {
        next_bit = 0;
        int const obs_rand = random->uniform(0,problems.size()-1);
        now_series = problems[obs_rand];
        next_answer = obs_rand;
    }
    if(next_bit < data_length)observation[0] = static_cast<double>(now_series[next_bit]);
    else observation[0] = 0;
    next_bit++;

#else
#ifdef ACTION_BUTTON
    if(!actioned)return 0.;//same observation, 0 reward
#endif
    int const obs_rand = random->uniform(0,problems.size()-1);
    bitset<data_length> const obs = problems[obs_rand];
    for(int i = 0; i < number_of_observation_vars; ++i)
    {
        observation[i] = static_cast<double>(obs[i]);
    }
    next_answer = obs.count();
#endif

    return static_cast<double>(reward);
}


double Count_Minority::restart()
{
    ++trial;
    next_answer = 0;
    for(int i=0; i<number_of_observation_vars; ++i)
    {
        observation[i] = 0.;
    }
    next_bit = 0;

    return 0.;
}