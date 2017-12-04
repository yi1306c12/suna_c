#include<boost/python.hpp>
#include<boost/python/suite/indexing/vector_indexing_suite.hpp>
#include<vector>
#include<new>
using std::vector;

#include"agents/Unified_Neural_Model.h"

#include"converters.hpp"


class UNF_python:public Unified_Neural_Model
{
public:
    using double_vector = vector<double>;

    UNF_python():Unified_Neural_Model(new State_of_Art_Random(time(NULL))){}

    
    void step_(vector<double> const& observation, double reward){
        Unified_Neural_Model::step(const_cast<double*>(&observation.front()),reward);
        laction = vector<double>(action,action+number_of_action_vars);
    }

    vector<double> const& last_action()
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
        .def("action",&UNF_python::last_action, return_internal_reference<>())
        .def("endEpisode", &UNF_python::endEpisode)
        .def("print", &UNF_python::print)
        .def("saveAgent", &UNF_python::saveAgent)
    ;

    to_python_converter<vector<double>, vector_to_pylist_converter<vector<double> > >();

    converter::registry::push_back(
        &pylist_to_vector_converter<vector<double> >::convertible,
        &pylist_to_vector_converter<vector<double> >::construct,
        boost::python::type_id<vector<double> >()
    );

    class_<UNF_python::double_vector>("double_vector")
        .def(vector_indexing_suite<UNF_python::double_vector>())
    ;
}

//http://d.hatena.ne.jp/moriyoshi/20091214/1260779899