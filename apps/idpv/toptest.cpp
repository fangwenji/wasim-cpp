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
  BTOR2Encoder btor_parser("../design/idpv-test/top.btor2", sts);
  std::cout << sts.trans()->to_string() << std::endl;


  smt::SmtLibReader smtlib_reader(solver);
  smtlib_reader.parse("../design/idpv-test/top.smt2");


  auto ret = smtlib_reader.lookup_symbol("main::1::O!0@1#1");
  std::cout << ret -> to_string() << std::endl;
  
  
  
  return 0;
}