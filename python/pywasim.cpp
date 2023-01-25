#include <boost/python.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include <framework/ts.h>

// CHECK url: 
//   https://cs.brown.edu/~jwicks/boost/libs/python/doc/tutorial/doc/html/python/object.html
//   https://www.qiniu.com/qfans/qnso-58096040
//   https://stackoverflow.com/questions/15842126/feeding-a-python-list-into-a-function-taking-in-a-vector-with-boost-python
//   https://wiki.python.org/moin/boost.python
//   https://py3c.readthedocs.io/en/latest/reference.html
//   

namespace wasim {
  /*
    * This is a wrapper class for all the exceptions
    * that will be thrown from the Python code.
    */
  struct PyWASIMException : public boost::exception
  {
      PyObject* exception;
      std::string message;

      PyWASIMException(PyObject* ex, const std::string& msg)
          : exception(ex)
          , message(msg)
      {}

      PyWASIMException(PyObject* ex, const char* msg)
          : exception(ex)
          , message(msg)
      {}

      virtual ~PyWASIMException() throw () {}
  };

  void translateWASIMException(PyWASIMException const& ex) {
    PyErr_SetString(ex.exception, ex.message.c_str());
  }

  boost::multiprecision::cpp_int pyint_to_cppint(const  boost::python::object& l)
  {
      PyObject* pyobj = l.ptr();
      PyObject* pystr = PyObject_Str(pyobj);
      std::string lstr(PyUnicode_AsUTF8(pystr));
      Py_DECREF(pystr);

      return boost::multiprecision::cpp_int(lstr);
  }

  std::string pystr_to_string(const boost::python::object& l)
  {
      PyObject* pyobj = l.ptr();
      PyObject* pystr = PyObject_Str(pyobj);
      std::string lstr(PyUnicode_AsUTF8(pystr));
      Py_DECREF(pystr);
      return lstr;
  }

  boost::python::object cppint_to_pyint(const boost::multiprecision::cpp_int& i)
  { 
      std::string si = i.str();
      return string_to_pyint(si);
  }

  boost::python::object string_to_pyint(const std::string& s)
  {
      PyObject* oi = PyLong_FromString((char*)s.c_str(), NULL, 0);
      boost::python::object pi(boost::python::handle<>(boost::python::borrowed(oi)));
      return pi;
  }

  bool is_py_int_or_long(const boost::python::object& l)
  {
      PyObject* pyobj = l.ptr();
      return PyLong_Check(pyobj);
  }



// ----------------------------------------------------------------------


  struct NodeType {
    smt::Sort sort;

    NodeType()  { throw PyWASIMException(PyExc_RuntimeError, "Cannot create NodeType directly. Use the context object."); }
    NodeType(const smt::Sort& ptr) : sort(ptr) { }
    NodeType(const NodeType& nr) : sort(nr.sort) { }

    smt::SortKind sortkind() const {return sort->get_sort_kind(); }
    int bitwidth() const { return sort->get_width();  }
    int arity() const { return sort->get_arity(); }

    bool isBool() const { return sort->get_sort_kind() == smt::SortKind::BOOL; }
    bool isBitvector() const { return sort->get_sort_kind() == smt::SortKind::BV; }
    bool isArray() const { return sort->get_sort_kind() == smt::SortKind::ARRAY; }

    std::string to_string() const { return sort->to_string(); }
  };


  struct NodeRef {
    smt::Term node;
    smt::SmtSolver solver;

    NodeRef() { throw PyWASIMException(PyExc_RuntimeError, "Cannot create Term directly. Use the context object."); }

    NodeRef(const smt::Term& ptr, const smt::SmtSolver & sptr) : 
      node(ptr), solver(sptr) { }

    NodeRef(const NodeRef& nr) : node(nr.node), solver(nr.solver) { }

    NodeRef::~NodeRef()  { }

    NodeRef NodeRef::operator=(const NodeRef& other)
    {
        if(this != &other) { 
            node = other.node;
            solver = other.solver;
        }
        return *this;
    }

    std::string to_string() const { return node->to_string(); }
    NodeType * getType() const { return new NodeType(node->get_sort()); }
    
    /*TODO : operators */

    NodeRef * substitute(boost::python::dict d) const { /*TODO*/ }

  };

  /* TODO : ts */

  /* TODO : executor */

  /* TODO : tracemgr */

  /* TODO : TraverseBranchingNode */
  
  /* TODO : TraverseBranchingNode */

} // end namespace wasim

BOOST_PYTHON_MODULE(pywasim)
{
  using namespace boost::python;
  using namespace wasim;
  
  class_<NodeType>("NodeType")
    .add_property("bitwidth", &NodeType::bitwidth)
    .add_property("arity", &NodeType::arity)
    .def("isBool", &NodeType::isBool)
    .def("isBitvector", &NodeType::isBitvector)
    .def("isArray", &NodeType::isArray)
    .def("__repr__", &NodeType::to_string)
  ;



}
