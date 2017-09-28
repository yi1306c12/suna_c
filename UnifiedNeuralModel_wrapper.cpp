#include<boost/python.hpp>
#include<boost/foreach.hpp>
#include<boost/range/value_type.hpp>

#include"agents/Unified_Neural_Model.h"

#include<vector>
#include<new>
using std::vector;


class UNF_python:public Unified_Neural_Model
{
public:
    typedef vector<double> double_vector;

    UNF_python():Unified_Neural_Model(new State_of_Art_Random(time(NULL))){}

    /*
    double_vector const& step(double_vector& observation){
        const double reward = 0;
        double* const v = Unified_Neural_Model::step(&observation.front(), reward);
        return double_vector(v, v+number_of_action_vars);
    }
    */
};


template<typename T>
class vector_to_pylist_converter
{
public:
    typedef T native_type;

    static PyObject* convert(native_type const& v)
    {
        namespace py = boost::python;
        py::list retval;
        BOOST_FOREACH(typename boost::range_value<native_type>::type i, v)
        {
            retval.append(py::object(i));
        }
        return py::incref(retval.ptr());
    }
};


template<typename T>
class pylist_to_vector_converter
{
public:
    typedef T native_type;

    static void* convertible(PyObject* pyo)
    {
        if(!PySequence_Check(pyo))return 0;
        return pyo;
    }

    static void construct(
        PyObject* pyo, 
        boost::python::converter::rvalue_from_python_stage1_data* data
    )
    {
        namespace py = boost::python;
        native_type* storage = new(reinterpret_cast<py::converter::rvalue_from_python_storage<native_type>*>(data)->storage.bytes) native_type();
        for (py::ssize_t i = 0, l = PySequence_Size(pyo); i < l; ++i)
        {
            storage->push_back(
                py::extract<typename boost::range_value<native_type>::type>
                (
                    PySequence_GetItem(pyo,i)
                )
            );
        }
        data->convertible = storage;
    }
};


BOOST_PYTHON_MODULE(unified_neural_model)
{
    using namespace boost::python;

    class_<UNF_python>("unified_neural_model")
        //.def("UNF_python", &UNF_python::UNF_python)
        .def("init", &UNF_python::init)
        //.def("step", (UNF_python::double_vector const&(UNF_python::*)(UNF_python::double_vector&))&UNF_python::step, return_value_policy<copy_const_reference>())
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