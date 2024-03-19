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
  
  // we want to see
  // sts.trans():  (= ret.next (bvadd a b))
  // ret_next_update_function: (bvadd a b)

  std::cout << "---------------------------C++ smtlib2---------------------------" << std::endl;
  
  auto a = sts.lookup("a");
  auto ret_next = sts.lookup("ret.next");

  // auto ret_1 = sts.named_terms();
  // std::cout<<"--------1: "<<ret_next<<std::endl;

  // auto ret_next_update_function = sts.state_updates().find(ret_next);
  // if (ret_next_update_function != sts.state_updates().end()) {
  //   auto ret_next_update_function1 = ret_next_update_function->second;
  //     // 使用 ret_next_update_function
  //   } else {
  //     // 处理 ret_next 不存在的情况
  //     std::cout<<"no exist"<<std::endl;
  // }
  
  smt::SmtLibReader smtlib_reader(solver);
  smtlib_reader.parse("../design/idpv-test/t1c.smt2");

  auto smt_a = smtlib_reader.lookup_symbol("f::a!0@1#1");
  auto smt_b = smtlib_reader.lookup_symbol("f::b!0@1#1");
  std::cout << smt_a -> to_string() << std::endl;
  std::cout << smt_b -> to_string() << std::endl;
  // if (f != nullptr) {
  //   std::cout << f->to_string() << std::endl;
  // }

  // //  a == |f::a!0@1#1|
  // //  b == |f::b!0@1#1|
  // //  ret_next_update_function != f

  // // 1. construct these SMT formulas
  // auto c1 = solver->make_term(smt::Equal, a, smt_a);
  // ...;

  // // 2. 
  // solver->assert_formula(c1);
  // ...;

  // auto res = solver->check_sat();
  // res.is_sat();


  return 0;
}
