#include <chrono>
#include "assert.h"
#include "config/testpath.h"
#include "frontend/btor2_encoder.h"
#include "framework/symsim.h"
#include "framework/ts.h"
#include "smt-switch/boolector_factory.h"

using namespace wasim;
using namespace smt;

int main() {


  SmtSolver solver = BoolectorSolverFactory::create(false);

  solver->set_logic("QF_UFBV");
  solver->set_opt("incremental", "true");
  solver->set_opt("produce-models", "true");
  solver->set_opt("produce-unsat-assumptions", "true");

  TransitionSystem sts(solver);
  BTOR2Encoder btor_parser("../design/test/adder.btor2", sts);

  std::cout << sts.trans()->to_string() << std::endl;
  
  /*------------------------------simulation--------------------------------*/
  sim.init();

    // timed assertion
  std::string my_assertion = "out@2 == a@0 + b@1";  // var with bit width format: (out@2)[3:0], (out@2)[0+:4]

  tac::TimedAssertionChecker test_tac(my_assertion, sts, sim, solver);

  bool rst_en0 = 0;    //if input signal have not rst, give 0;  if give 1 -> rst ≡ 0, if give 0 -> rst = rst1,2,3...(symbolic)
  test_tac.sim_max_step(rst_en0);
  test_tac.print_term_map();
  test_tac.make_assertion_term();
  test_tac.assert_formula();
  test_tac.check_sat();

  return 0;
}


