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
  BTOR2Encoder btor_parser("../design/idpv-test/2cyclemul.btor2", sts); 
  std::cout << sts.trans()->to_string() << std::endl;

  SymbolicExecutor executor(sts, solver);

  /* initial state assignment */
  assignment_type in={};
  auto initial_state = executor.convert(in);
    
  executor.init(initial_state);
  executor.set_input(executor.convert({{"A",1},{"B",1}}),{});
  executor.sim_one_step();

  auto s1 = executor.get_curr_state();
  std::cout<< s1.print();

  executor.set_input(executor.convert({{"enable",1}}),{});
  executor.sim_one_step();
  
  auto s2 = executor.get_curr_state();
  std::cout << s2.print();

  return 0;
}
