#include<boost/python.hpp>
#include<boost/python/numpy.hpp>
namespace p = boost::python;
namespace np = p::numpy;

#include<stdexcept>
using std::runtime_error;

#include<time.h>//clock
#include<unistd.h>//getpid

#include<vector>
using std::vector;

#include"Module.h"

class Module_python
{
    np::dtype const dtype = np::dtype::get_builtin<double>();
    p::tuple const stride = p::make_tuple(sizeof(double));

    Module* module;
    vector<double> output;

public:
    Module_python(){}

    Module_python(Module_python& const brother)
    {
        
    }

    void init(int const number_of_inputs, int const number_of_outputs)
    {
        module = new Module(number_of_inputs, number_of_outputs, INITIAL_ALLOCATION_LENGTH);
        output = vector<double>(number_of_outputs));
    }

    np::ndarray process(np::ndarray input)
    {
        if(input.get_nd() != 1)throw runtime_error("input must be 1-dim");


    }

    int const get_number_of_neurons(void)
    {
        return module->number_of_neurons;
    }
};

BOOST_PYTHON_MODULE(module)
{
    using namespace boost::python;
    Py_Initialize();
    np::initialize();
    
    Module::setRandom(new State_of_Art_Random(
        static_cast<unsigned int>(clock())+static_cast<unsigned int>(gedpid())
    );

    class<Module_python>("module")
        .def()
}