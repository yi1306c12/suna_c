#include "Gym_Wrapper.hpp"

#include<string>
#include<fstream>
#include<streambuf>
#include<vector>
#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include <iostream>
#include <cmath>

namespace p = boost::python;
namespace np = p::numpy;
using namespace std;


Gym_Wrapper::Gym_Wrapper(const char* environment_name)
{
    Py_Initialize();
    np::initialize();

    cout << environment_name << endl; //debug

    global_namespace = p::import("__main__").attr("__dict__");
    global_namespace["environment_name"] = environment_name;//set environment_name(in python) = environment_name(in c++)

    try
    {
        ifstream ifs("environments/Gym_Wrapper.py");
        string script(
            (istreambuf_iterator<char>(ifs)),
            istreambuf_iterator<char>()
        );
        p::exec(script.c_str(),global_namespace);
        number_of_observation_vars = p::extract<int>(global_namespace["number_of_observation_vars"]);
        number_of_action_vars = p::extract<int>(global_namespace["number_of_action_vars"]);
    }
    catch(p::error_already_set& e)
    {
        PyErr_Print();
        throw e;
    }

    shape_action = p::make_tuple(number_of_action_vars);
    observation = new double[number_of_observation_vars];
    MAX_STEPS = 1000;
    trial = -1; 
}


void Gym_Wrapper::start(int& number_of_observation_vars, int& number_of_action_vars)
{
    number_of_observation_vars = this->number_of_observation_vars;
    number_of_action_vars = this->number_of_action_vars;

    restart();
}


double Gym_Wrapper::step(double* action)
{
    double reward = 0.;
    auto action_np = np::from_data(
        action,
        np::dtype::get_builtin<double>(),
        shape_action,
        p::make_tuple(sizeof(double)),
        p::object()
    );
    try
    {
        auto step = global_namespace["step"];
        reward = p::extract<double>(step(action_np));
    }
    catch(p::error_already_set& e)
    {
        PyErr_Print();
        throw e;
    }

    auto observation_np = global_namespace["observation"];
    set_observation(observation_np);
    return reward;
}


double Gym_Wrapper::restart(void)
{
    ++trial;

    try
    {
        auto restart = global_namespace["restart"];
        auto observation_np = restart();
        set_observation(observation_np);
    }
    catch(p::error_already_set)
    {
        PyErr_Print();
    }

    return 0.;
}


void Gym_Wrapper::set_observation(p::object observation_np)
{
    p::stl_input_iterator<double> begin(observation_np),end;
    vector<double> observation_vector(begin,end);
    for(int i=0; i < number_of_observation_vars; ++i)
    {
        observation[i] = observation_vector[i];
    }
}