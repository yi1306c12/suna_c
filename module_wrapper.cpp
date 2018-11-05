#include<boost/python.hpp>
#include<boost/python/numpy.hpp>
namespace p = boost::python;
namespace np = p::numpy;

#include<new>
#include<stdexcept>
using std::runtime_error;

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
        output(new double[number_of_outputs])
    {
        setRandom(new State_of_Art_Random(
            static_cast<unsigned int>(clock())+static_cast<unsigned int>(getpid())
        ));
    }
    np::ndarray process(np::ndarray input)
    {
        if(observation.get_nd() != 1)throw runtime_error("input must be 1-dimensional");

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
};


BOOST_PYTHON_MODULE(module)
{
    using namespace boost::python;
    Py_Initialize();
    np::initialize();

    class<module_python>("module", init<int, int>())
    //[Boost\.Python（日本語訳）](http://alpha.osdn.jp/devel/boost.python_ja.pdf)	
        .def("process", &module_python::process)
        .def("weightMutation", &module_python::weightMutation)//in Unified_Neural_Model::init(), do only structuralMutation but weightMutation
        .def("structuralMutation", &module_python::structuralMutation)
//        .def("clone", &module_python::clone)
    ;
}