#include<boost/python.hpp>
#include<boost/python/numpy.hpp>
namespace p = boost::python;
namespace np = p::numpy;

#include<new>
#include<stdexcept>
using std::runtime_error;
#include<exception>
using std::invalid_argument;

#include"agents/Unified_Neural_Model.h"


class UNF_python:public Unified_Neural_Model
{
    np::dtype const dtype = np::dtype::get_builtin<double>();
    p::tuple const stride = p::make_tuple(sizeof(double));

public:
    UNF_python(int number_of_actions, int number_of_observations):
        Unified_Neural_Model(new State_of_Art_Random(time(NULL)))
    {
        Unified_Neural_Model::init(number_of_observations, number_of_actions);
    }

    
    np::ndarray const step_(np::ndarray const & observation, double reward){
        if(observation.get_nd() != 1)throw runtime_error("observation must be 1-dimensional");

        p::tuple const shape = p::make_tuple(number_of_action_vars);//this should get out
        Unified_Neural_Model::step(reinterpret_cast<double *>(observation.get_data()),reward);

        return np::from_data(action, dtype, shape, stride, p::object());
    }
};


BOOST_PYTHON_MODULE(unified_neural_model)
{
    using namespace boost::python;
    Py_Initialize();
    np::initialize();

    class_<UNF_python>("unified_neural_model", init<int, int>())
        //.def("__init__", &UNF_python::UNF_python)
        //.def("init", &UNF_python::init)
        .def("step", &UNF_python::step_)
        .def("endEpisode", &UNF_python::endEpisode)
        .def("print", &UNF_python::print)
        .def("saveAgent", &UNF_python::saveAgent)
        .def("resetAgent", &UNF_python::resetAgent)
    ;

}

//https://qiita.com/termoshtt/items/81eeb0467d9087958f7f
//https://qiita.com/termoshtt/items/0103803c40331c77c727