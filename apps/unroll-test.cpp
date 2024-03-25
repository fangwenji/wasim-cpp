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
  BTOR2Encoder btor_parser("../design/idpv-test/t1v.btor2", sts);

  std::cout << sts.trans()->to_string() << std::endl;

  SymbolicExecutor executor(sts, solver);
  std::cout << "aaaaaaaaaaaaaaaaaaaa" << std::endl;

  /* initial state assignment */
  assignment_type initial_state1;
  initial_state1["a"] = 1;
  initial_state1["b"] = 1;
  auto initial_state = executor.convert(initial_state1);
    
    
  executor.init(initial_state);

  executor.set_input(
      executor.convert({{"rst", 0}, {"input_var2" , 1}}), {});
  executor.sim_one_step();

  auto s1 = executor.get_curr_state();
  auto expr = s1.get_sv().at(sts.lookup("a"));


  return 0;
}


