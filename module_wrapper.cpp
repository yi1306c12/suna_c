#include<boost/python.hpp>
#include<boost/python/numpy.hpp>
namespace p = boost::python;
namespace np = p::numpy;

#include<new>
#include<stdexcept>
using std::runtime_error;
#include<exception>
using std::invalid_argument;

//for random seed
#include<unistd.h>//getpid
#include<time.h>//clock


#include"agents/modules/Module.h"

class module_python:public Module
{
    np::dtype const dtype = np::dtype::get_builtin<double>();
    p::tuple const stride = p::make_tuple(sizeof(double));
    p::tuple const shape;

public:
    module_python(int const number_of_inputs, int const number_of_outputs):
        Module(number_of_inputs, number_of_outputs, INITIAL_ALLOCATION_LENGTH),
        shape(p::make_tuple(number_of_outputs))
    {
        setRandom(new State_of_Art_Random(
            static_cast<unsigned int>(clock())+static_cast<unsigned int>(getpid())
        ));

        output = new double[number_of_outputs];
    }
    np::ndarray process(np::ndarray const & input)
    {
        if(input.get_nd() != 1)throw invalid_argument("input must be 1-dimensional");
        if(input.get_dtype() != np::dtype::get_builtin<double>())throw invalid_argument("input must be numpy.float64");

        Module::process(reinterpret_cast<double *>(input.get_data()), output);
        return np::from_data(output, dtype, shape, stride, p::object());
    }
    void structuralMutation(int const number_of_mutation)
    {
        for(int k=0; k < number_of_mutation; ++k)
        {
            Module::structuralMutation();
        }
        updatePrimerList();
    }
    //for debug
    np::ndarray check_input(np::ndarray const & input, int unsigned const number_of_inputs)
    {
        p::tuple const input_shape = p::make_tuple(number_of_inputs);
        if(input.get_nd() != 1)throw invalid_argument("input must be 1-dimensional");
        return np::from_data(reinterpret_cast<double *>(input.get_data()), dtype, input_shape, stride, p::object());
    }
};

class module_python_suite : public p::pickle_suite
{
    //picklable
    //boost python pickle c++
    //(https://gist.github.com/seanevans/04b61f58a321c46fec94)	
/*
    static p::tuple getstate(p::object w_obj)
    {
        Module const & w = p::extract<Module const &>(w_obj)()
        return p::make_tuple(
            w_obj.attr("__dict__"), w.id, 
        )
    }
*/
}

BOOST_PYTHON_MODULE(module_wrapper)
{
    using namespace boost::python;
    Py_Initialize();
    np::initialize();

    class_<module_python, boost::noncopyable>("module", init<int, int>())
        //.enable_pickling()
        //.def_pickle(module_python_suite())
    //[Boost\.Python（日本語訳）](http://alpha.osdn.jp/devel/boost.python_ja.pdf)
        .def("process", &module_python::process)
        .def("weightMutation", &module_python::weightMutation)//in Unified_Neural_Model::init(), do only structuralMutation but weightMutation
        .def("structuralMutation", &module_python::structuralMutation)
//        .def("clone", &module_python::clone)
        .def("check_input", &module_python::check_input)
    ;
}