#include"Reinforcement_Environment.h"

#include <boost/python.hpp>

class Gym_Wrapper : public Reinforcement_Environment
{
    boost::python::object global_namespace;
    boost::python::tuple shape_action;
    void set_observation(boost::python::object observation_np);
public:
    Gym_Wrapper(const char* environment_name);
    ~Gym_Wrapper();

    void start(int& number_of_observation_vars, int& number_of_action_vars);
    double step(double* action);
    double restart();
};