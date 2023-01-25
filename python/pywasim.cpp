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

  boost::python::object string_to_pyint(const std::string& s)
  {
      PyObject* oi = PyLong_FromString((char*)s.c_str(), NULL, 0);
      boost::python::object pi(boost::python::handle<>(boost::python::borrowed(oi)));
      return pi;
  }

  boost::python::object cppint_to_pyint(const boost::multiprecision::cpp_int& i)
  { 
      std::string si = i.str();
      return string_to_pyint(si);
  }


  bool is_py_int_or_long(const boost::python::object& l)
  {
      PyObject* pyobj = l.ptr();
      return PyLong_Check(pyobj);
  }



// ----------------------------------------------------------------------


  struct NodeType {

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
    size_t hash() const {return sort->hash();}

  protected:
    smt::Sort sort;
  };


  struct NodeRef {

    NodeRef() { throw PyWASIMException(PyExc_RuntimeError, "Cannot create Term directly. Use the context object."); }

    NodeRef(const smt::Term& ptr, const smt::SmtSolver & sptr) : 
      node(ptr), solver(sptr) { }

    NodeRef(const NodeRef& nr) : node(nr.node), solver(nr.solver) { }

    ~NodeRef()  { }

    NodeRef operator=(const NodeRef& other)
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
    NodeRef * arg(size_t n) const { /*TODO*/ }
    size_t hash() const { return node->hash();  }

  protected:

    smt::Term node;
    smt::SmtSolver solver;

    unsigned bvwidth() const { 
      if ( node->get_sort()->get_sort_kind() != smt::SortKind::BV) return 0;
      return node->get_sort()->get_width(); }
    bool isbool() const { return node->get_sort()->get_sort_kind() == smt::SortKind::BOOL; }
    // ---------------------------------------------------------------------- //
    NodeRef* _unOp(smt::PrimOp opBool , smt::PrimOp opBv, const char* opName) const
    {   
        try{
          if (isbool())
            return new NodeRef(solver->make_term(opBool, node), solver);
          if (bvwidth())
            return new NodeRef(solver->make_term(opBv, node), solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        } 

        throw PyWASIMException(PyExc_TypeError, std::string("Incorrect type for ") + 
                              opName);
        return NULL;
    }

    NodeRef* _binOp(
        smt::PrimOp boolOp, 
        smt::PrimOp bvOp, 
        const char* opName,
        NodeRef* other) const
    {
        try{
          if (isbool())
            return new NodeRef(solver->make_term(boolOp, node, other->node), solver);
          if (bvwidth())
            return new NodeRef(solver->make_term(bvOp, node, other->node), solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        } 
        throw PyWASIMException(PyExc_TypeError, std::string("Incorrect type for ") + 
                              opName);
        return NULL;
    }

    NodeRef* _binOp(smt::PrimOp op, const char* opName, NodeRef* other) const
    {
        try{
          return new NodeRef(solver->make_term(op, node, other->node), solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        }
        throw PyWASIMException(PyExc_TypeError, std::string("Incorrect type for ") + 
                              opName);
        return NULL;
    }

    NodeRef* _binOpL(smt::PrimOp op, const char* opName, int r) const
    {
      auto sz = bvwidth();
      if(sz != 0) {
        try{
          auto cterm = solver->make_term(r, solver->make_sort(smt::SortKind::BV, sz));
          return new NodeRef(solver->make_term(op, node, cterm), solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        }
      } else if (isbool()) {
        try{
          auto cterm = solver->make_term((bool)(r));
          return new NodeRef(solver->make_term(op, node, cterm), solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        }
      }
      throw PyWASIMException(PyExc_TypeError, std::string("Incorrect type for ") + 
                            opName);
      return NULL;
    }

    NodeRef* _binOpR(smt::PrimOp op, const char* opName, int r) const
    {
      auto sz = bvwidth();
      if(sz != 0) {
        try{
          auto cterm = solver->make_term(r, solver->make_sort(smt::SortKind::BV, sz));
          return new NodeRef(solver->make_term(op, cterm, node ), solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        }
      } else if (isbool()) {
        try{
          auto cterm = solver->make_term((bool)(r));
          return new NodeRef(solver->make_term(op, cterm, node), solver);
        } catch (SmtException e) {
          throw PyWASIMException(PyExc_TypeError, e.what());
        }
      }
      throw PyWASIMException(PyExc_TypeError, std::string("Incorrect type for ") + 
                            opName);
      return NULL;
    }

    static NodeRef* triOp(smt::PrimOp Op, const char* opName,
                   NodeRef * cond, NodeRef * trueExp, NodeRef * falseExp)
    {   
      try{
        return new NodeRef(cond->solver->make_term(Op, cond->node, 
          trueExp->node, falseExp->node), cond->solver);
      } catch (SmtException e) {
        throw PyWASIMException(PyExc_TypeError, e.what());
      }
      throw PyWASIMException(PyExc_TypeError, std::string("Incorrect type for ") + 
                            opName);
      return NULL;
    }

    static NodeRef* _extractOp(const NodeRef* obj, int beg, int end)
    {
      if (obj->bvwidth() == 0)
        throw PyWASIMException(PyExc_TypeError, "Incorrect type for SLICE" );
      smt::Op exop(smt::PrimOp::Extract, beg, end);
      try{
        return new NodeRef(obj->solver->make_term(exop, obj->node), obj->solver);
      } catch (SmtException e) {
        throw PyWASIMException(PyExc_TypeError, e.what());
      }
      throw PyWASIMException(PyExc_TypeError, "Incorrect type for SLICE" );
      return NULL;
    }

  }; // end of NodeRef

  /* TODO : State */
  struct StateRef {
    StateRef() { throw PyWASIMException(PyExc_RuntimeError, "Cannot create State directly. Use the context object."); }
    StateRef(const StateRef & other) : sptr(other.sptr) , solver(other.solver ){ }
    StateRef(StateAsmpt * ptr, smt::SmtSolver osolver) : sptr(ptr), solver(osolver) { }

    StateRef * clone() const {
      return new StateRef(new StateAsmpt(*sptr), solver);
    }

    // test the type of the return list and see how that works
    boost::python::list get_assumptions() const { /*TODO*/ 
      boost::python::list ret;
      for (const auto & a : sptr->get_assumptions())
        ret.append( new NodeRef(a, solver) );
      return ret;
    }

    // NodeRef* get_assumption(size_t idx) const { /*TODO*/ } // can we return a list of NodeRef?
    void set_assumptions(boost::python::list l) { /*TODO : use extract */ }

    boost::python::list get_assumption_interps() const { /*TODO*/ }
    void set_assumption_interps(boost::python::list l) { /*TODO*/ }

    boost::python::list get_var() const {} // list of string
    NodeRef* get_term(const std::string & name) const { /*TODO*/ }
    void set_sv(boost::python::dict v) { /*TODO*/ }

    protected:
      std::shared_ptr<StateAsmpt> sptr;
      smt::SmtSolver solver;
  };

  /* TODO : ts */
  // terms() return all named_terms boost::python::dict
  // lookup() return a term by its name

  /* TODO : executor */

  /* TODO : tracemgr */

  /* TODO : TraverseBranchingNode */

  /* TODO : TraverseBranchingNode */

} // end namespace wasim

BOOST_PYTHON_MODULE(pywasim)
{
  using namespace boost::python;
  using namespace wasim;

  // setup the exception translator.
  register_exception_translator
      <PyWASIMException>(translateWASIMException);

  class_<NodeType>("NodeType")
    .add_property("bitwidth", &NodeType::bitwidth)
    .add_property("arity", &NodeType::arity)
    .def("isBool", &NodeType::isBool)
    .def("isBitvector", &NodeType::isBitvector)
    .def("isArray", &NodeType::isArray)
    .def("__repr__", &NodeType::to_string)
    .def("__hash__", &NodeType::hash)
  ;

  class_<StateRef>("StateRef")
    .def("get_assumptions",&StateRef::get_assumptions)
    .def("set_assumptions",&StateRef::set_assumptions)
  ;

}
