#include<boost/python.hpp>
#include<boost/python/suite/indexing/vector_indexing_suite.hpp>
#include<vector>
#include<new>
using std::vector;

#include"environments/Reinforcement_Environment.h"
#include"environments/Double_Cart_Pole.h"
#include"random/State_of_Art_Random.h"

#include"converters.hpp"


class doublecartpole_python:public Double_Cart_Pole
{
public:
    using double_vector = vector<double>;
    
//  typedef vector<double> double_vector;
    int number_of_observation_vars=-1;
    int number_of_action_vars=-1;

    doublecartpole_python():Double_Cart_Pole(new State_of_Art_Random(time(NULL)))
    {
        start(number_of_observation_vars,number_of_action_vars);
    }

    const double step_(vector<double> const& action)
    {
       return Double_Cart_Pole::step(const_cast<double*>(&action.front()));
    }

    vector<double> const& reset()
    {
        restart();
        return last_observation();
    }

    vector<double> const& last_observation()
    {
        lobservation = vector<double>(observation,observation+number_of_observation_vars);
        return lobservation;
    }
private:
    double_vector lobservation;
};


BOOST_PYTHON_MODULE(environments)
{
    using namespace boost::python;

    class_<doublecartpole_python>("double_cart_pole")
        .def("reset",&doublecartpole_python::reset,return_internal_reference<>())
        .def("step",&doublecartpole_python::step_)
        .def_readonly("observations",&doublecartpole_python::number_of_observation_vars)
        .def_readonly("actions",&doublecartpole_python::number_of_action_vars)
        .def_readonly("MAX_STEPS",&doublecartpole_python::MAX_STEPS)
        .def("last_observation",&doublecartpole_python::last_observation,return_internal_reference<>())
        .def_readonly("trial",&doublecartpole_python::trial)
    ;

    to_python_converter<vector<double>, vector_to_pylist_converter<vector<double> > >();

    converter::registry::push_back(
        &pylist_to_vector_converter<vector<double> >::convertible,
        &pylist_to_vector_converter<vector<double> >::construct,
        boost::python::type_id<vector<double> >()
    );

    class_<doublecartpole_python::double_vector>("double_vector")
        .def(vector_indexing_suite<doublecartpole_python::double_vector>())
    ;
}