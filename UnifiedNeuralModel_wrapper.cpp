#include<boost/python.hpp>
#include<boost/range/value_type.hpp>

#include"agents/Unified_Neural_Model.h"

#include<vector>
#include<new>
using std::vector;

#include"converters.hpp"

class UNF_python:public Unified_Neural_Model
{
public:
    typedef vector<double> double_vector;

    UNF_python():Unified_Neural_Model(new State_of_Art_Random(time(NULL))){}

    
    void step_(double_vector const& observation, double reward){
        Unified_Neural_Model::step(const_cast<double*>(&observation.front()),reward);
        laction = double_vector(action,action+number_of_action_vars);
    }

    double_vector const& last_action()
    {
        return laction;
    }
private:
    double_vector laction;    
};


BOOST_PYTHON_MODULE(unified_neural_model)
{
    using namespace boost::python;

    class_<UNF_python>("unified_neural_model")
        //.def("__init__", &UNF_python::UNF_python)
        .def("init", &UNF_python::init)
        .def("step", &UNF_python::step_)
        .def("action",&UNF_python::last_action, return_value_policy<copy_const_reference>())
        .def("endEpisode", &UNF_python::endEpisode)
        .def("print", &UNF_python::print)
        .def("saveAgent", &UNF_python::saveAgent)
    ;

    to_python_converter<UNF_python::double_vector, vector_to_pylist_converter<UNF_python::double_vector> >();

    converter::registry::push_back(
        &pylist_to_vector_converter<UNF_python::double_vector>::convertible,
        &pylist_to_vector_converter<UNF_python::double_vector>::construct,
        boost::python::type_id<UNF_python::double_vector>()
    );
}

//http://d.hatena.ne.jp/moriyoshi/20091214/1260779899