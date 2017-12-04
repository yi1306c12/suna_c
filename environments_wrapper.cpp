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
};


BOOST_PYTHON_MODULE(environments)
{
    using namespace boost::python;

    class_<doublecartpole_python>("double_cart_pole")
        .def("restart",&doublecartpole_python::restart)
        .def("step",&doublecartpole_python::step_)
    ;

    converter::registry::push_back(
        &pylist_to_vector_converter<doublecartpole_python::double_vector>::convertible,
        &pylist_to_vector_converter<doublecartpole_python::double_vector>::construct,
        boost::python::type_id<doublecartpole_python::double_vector>()
    );
}