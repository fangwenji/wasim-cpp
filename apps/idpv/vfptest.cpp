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
    BTOR2Encoder btor_parser("../design/idpv-test/vfpmul.btor2", sts);
    std::cout << sts.trans()->to_string() << std::endl;

    auto src1 = sts.lookup("flout_a");
    auto src2 = sts.lookup("flout_b");
    auto res = sts.lookup("flout_c");
    
    SymbolicExecutor executor(sts,solver);
    assignment_type initial_state1={};
    auto initial_state = executor.convert(initial_state1);
    executor.init(initial_state);
    executor.set_input(executor.convert({{"flout_a",00000000000000000000000000000001},{"flout_b",00000000000000000000000000000001}}),{});
    executor.sim_one_step();
    auto s1 = executor.get_curr_state();
    std::cout <<s1.print();

    return 0;
    

}