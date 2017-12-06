#include<boost/python.hpp>
#include<boost/numpy.hpp>
namespace p = boost::python;
namespace np = boost::numpy;

#include<new>
#include<stdexcept>
using std::runtime_error;

//#include"environments/Reinforcement_Environment.h"
#include"environments/Double_Cart_Pole.h"
#include"random/State_of_Art_Random.h"


class doublecartpole_python:public Double_Cart_Pole
{
    np::dtype const dtype = np::dtype::get_builtin<double>();
    p::tuple const stride = p::make_tuple(sizeof(double));
    
public:
    int number_of_observation_vars=-1;
    int number_of_action_vars=-1;

    doublecartpole_python():Double_Cart_Pole(new State_of_Art_Random(time(NULL)))
    {
        start(number_of_observation_vars,number_of_action_vars);
    }

    const double step_(np::ndarray action, double reward)
    {
        if(action.get_nd() != 1)throw runtime_error("action must be 1-dimensional");
        return Double_Cart_Pole::step(reinterpret_cast<double*>(action.get_data()));
    }

    np::ndarray reset()
    {
        restart();
        return last_observation();
    }

    np::ndarray last_observation()
    {
        p::tuple const shape = p::make_tuple(number_of_observation_vars);//shuold get out
        return np::from_data(observation, dtype, shape, stride, p::object());
    }
};


BOOST_PYTHON_MODULE(environments)
{
    using namespace boost::python;
    Py_Initialize();
    np::initialize();

    class_<doublecartpole_python>("double_cart_pole")
        .def("reset",&doublecartpole_python::reset)
        .def("step",&doublecartpole_python::step_)
        .def_readonly("observations",&doublecartpole_python::number_of_observation_vars)
        .def_readonly("actions",&doublecartpole_python::number_of_action_vars)
        .def_readonly("MAX_STEPS",&doublecartpole_python::MAX_STEPS)
        .def("last_observation",&doublecartpole_python::last_observation)
        .def_readonly("trial",&doublecartpole_python::trial)
    ;
}