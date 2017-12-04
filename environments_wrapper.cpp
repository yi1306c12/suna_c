#include<boost/python.hpp>
#include<boost/foreach.hpp>
#include<boost/range/value_type.hpp>

#include"environments/Reinforcement_Environment.h"
#include"environments/Double_Cart_Pole.h"

#include"random/State_of_Art_Random.h"

#include"converters.hpp"

#include<vector>
#include<new>
using std::vector;
#include<iterator>

class doublecartpole_python:public Double_Cart_Pole
{
public:

    typedef vector<double> double_vector;
    int number_of_observation_vars=-1;
    int number_of_action_vars=-1;

    doublecartpole_python():Double_Cart_Pole(new State_of_Art_Random(time(NULL)))
    {
        start(number_of_observation_vars,number_of_action_vars);
    }

    double step_(double_vector const& action)
    {
        return Double_Cart_Pole::step(const_cast<double*>(&action.front()));
    }

    double_vector const& reset()
    {
        restart();
        return last_observatoin()
    }

    double_vector const& last_observation()
    {
        return double_vector(std::begin(observation),std::end(observation));
    }
};


BOOST_PYTHON_MODULE(environments)
{
    using namespace boost::python;

    class_<doublecartpole_python>("double_cart_pole")
        .def("reset",&doublecartpole_python::reset,return_value_policy<copy_const_reference>())
        .def("step",&doublecartpole_python::step_)
        .def_readonly("observations",&doublecartpole_python::number_of_observation_vars)
        .def_readonly("actions",&doublecartpole_python::number_of_action_vars)
        .def_readonly("MAX_STEPS",&doublecartpole_python::MAX_STEPS)
        .def("last_observation",&doublecartpole_python::last_observation,return_value_policy<copy_const_reference>())
    ;

    to_python_converter<doublecartpole_python::double_vector, vector_to_pylist_converter<doublecartpole_python::double_vector> >();

    converter::registry::push_back(
        &pylist_to_vector_converter<doublecartpole_python::double_vector>::convertible,
        &pylist_to_vector_converter<doublecartpole_python::double_vector>::construct,
        boost::python::type_id<doublecartpole_python::double_vector>()
    );
}