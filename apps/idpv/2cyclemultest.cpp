#include <chrono>
#include "assert.h"
#include "config/testpath.h"
#include "frontend/btor2_encoder.h"
#include "framework/symsim.h"
#include "framework/ts.h"
#include "smt-switch/boolector_factory.h"
#include "smt-switch/smtlib_reader.h"

using namespace wasim;
using namespace smt;

int main() {
  SmtSolver solver = BoolectorSolverFactory::create(false);

  solver->set_logic("QF_UFBV");
  solver->set_opt("incremental", "true");
  solver->set_opt("produce-models", "true");
  solver->set_opt("produce-unsat-assumptions", "true");


  TransitionSystem sts(solver);
  BTOR2Encoder btor_parser("../design/idpv-test/2cyclemul.btor2", sts); 
  std::cout << sts.trans()->to_string() << std::endl;

  auto a = sts.lookup("A");
  auto b = sts.lookup("B");
  auto ret = sts.lookup("result");

  std::cout<<a->to_string()<<std::endl;
  std::cout<<b->to_string()<<std::endl;
  std::cout<<ret->to_string()<<std::endl;

  // //这部分可以实现unroll cycle
  // SymbolicExecutor executor(sts, solver);

  // /* initial state assignment */
  // assignment_type in={};
  // auto initial_state = executor.convert(in);
    
  // executor.init(initial_state);
  // executor.set_input(executor.convert({{"A",1},{"B",1}}),{});
  // executor.sim_one_step();

  // auto s1 = executor.get_curr_state();
  // // std::cout<< s1.print();

  // executor.set_input(executor.convert({{"enable",1}}),{});
  // executor.sim_one_step();
  
  // auto s2 = executor.get_curr_state();
  // // std::cout << s2.print();

  smt::SmtLibReader smtlib_reader(solver);
  smtlib_reader.parse("../design/idpv-test/concat_error.smt2");
  auto in_a = smtlib_reader.lookup_symbol("__mulsf3::a!0@1#1");
  auto in_b = smtlib_reader.lookup_symbol("__mulsf3::b!0@1#1");
  auto in_ret = smtlib_reader.lookup_symbol("__mulsf3::$tmp::return_value_mulsf3_classical!0@1#2");

  std::cout<<in_a->to_string()<<std::endl;
  std::cout<<in_b->to_string()<<std::endl;
  std::cout<<in_ret->to_string()<<std::endl;

  auto check_a = solver->make_term(smt::Equal,a,in_a);
  solver->assert_formula(check_a);
  

  auto check_b = solver->make_term(smt::Equal,b,in_b);
  solver->assert_formula(check_b);

  auto ret_extended = solver->make_term(smt::Op(smt::PrimOp::Sign_Extend, 32), ret);
  std::cout<<ret_extended->get_sort()<<std::endl;
  std::cout<<in_ret->get_sort()<<std::endl;
  

  auto check_ret = solver->make_term(smt::Equal,ret_extended,in_ret);
  Term not_equal = solver->make_term(Not, check_ret);
  TermVec assmpt {not_equal};
  auto res = solver->check_sat_assuming(assmpt);

  if(res.is_unsat()){
    std::cout<<"always equal"<<std::endl;
  } else if (res.is_sat())
  {
    std::cout<<"exist unequal"<<std::endl;
  }

  return 0;
}
