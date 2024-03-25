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

  std::cout << "---------------------------Verilog btor---------------------------" << std::endl;
  
  TransitionSystem sts(solver);
  BTOR2Encoder btor_parser("../design/idpv-test/t1v.btor2", sts);
  std::cout << sts.trans()->to_string() << std::endl;
  
  auto a = sts.lookup("a");
  auto b = sts.lookup("b");
  auto ret_next = sts.lookup("ret");
  auto ret_next_update_function = sts.state_updates().at(ret_next);
  std::cout << ret_next_update_function->to_string() << std::endl;

  std::cout << "---------------------------C++ smtlib2---------------------------" << std::endl;

  smt::SmtLibReader smtlib_reader(solver);
  smtlib_reader.parse("../design/idpv-test/t1c.smt2");

  auto smt_a = smtlib_reader.lookup_symbol("f::a!0@1#1");
  auto smt_b = smtlib_reader.lookup_symbol("f::b!0@1#1");
  auto smt_ret = smtlib_reader.lookup_symbol("goto_symex::return_value::f!0#1");
  std::cout << smt_a -> to_string() << std::endl;
  std::cout << smt_b -> to_string() << std::endl;
  std::cout << smt_ret -> to_string() << std::endl;

  // auto check_a = solver->make_term(smt::Equal, a, smt_a);
  // solver->assert_formula(check_a);
  // auto res_a = solver->check_sat();
  // if (res_a.is_sat()) {
  //   std::cout << "The formula regarding 'a' is satisfiable." << std::endl;
  // } else if (res_a.is_unsat()) {
  //   std::cout << "The formula regarding 'a' is unsatisfiable. Explanation: " << res_a.get_explanation() << std::endl;
  // } else if (res_a.is_unknown()) {
  //   std::cout << "The satisfiability of the formula regarding 'a' is unknown. Explanation: " << res_a.get_explanation() << std::endl;
  // }

  // auto check_b = solver->make_term(smt::Equal, b, smt_b);
  // solver->assert_formula(check_b);
  // auto res_b = solver->check_sat();
  // if (res_b.is_sat()) {
  //   std::cout << "The formula regarding 'b' is satisfiable." << std::endl;
  // } else if (res_b.is_unsat()) {
  //   std::cout << "The formula regarding 'b' is unsatisfiable. Explanation: " << res_b.get_explanation() << std::endl;
  // } else if (res_b.is_unknown()) {
  //   std::cout << "The satisfiability of the formula regarding 'b' is unknown. Explanation: " << res_b.get_explanation() << std::endl;
  // }

  auto check_ret = solver->make_term(smt::Equal, ret_next_update_function, smt_ret);
  Term not_equal = solver -> make_term(Not, check_ret);
  TermVec assumptions{ not_equal};
  auto res_ret = solver->check_sat_assuming(assumptions);

  if(res_ret.is_unsat()){
    std::cout << "always sat" << std::endl;
    
  } else if(res_ret.is_sat()){
    std::cout << "exist unsat" << std::endl;
  }

  return 0;
}

  // we want to see
  // sts.trans():  (= ret.next (bvadd a b))    yes
  // ret_next_update_function: (bvadd a b)     yes
  //  a == |f::a!0@1#1|                        yes
  //  b == |f::b!0@1#1|                        yes
  //  ret_next_update_function ！= f           ???