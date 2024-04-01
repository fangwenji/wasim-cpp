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

    // std::cout << "---------------------------Verilog btor---------------------------" << std::endl;
  
    TransitionSystem sts(solver);
    BTOR2Encoder btor_parser("../design/idpv-test/vmul16.btor2", sts);
    // std::cout << sts.trans()->to_string() << std::endl;

    // auto src1 = sts.lookup("floatA");
    // auto src2 = sts.lookup("floatB");
    // auto res = sts.lookup("product");
    // auto res_next = sts.state_updates().at(res);
    // std::cout << res_next->to_string() << std::endl;

    std::cout << "---------------------------C++ smtlib2---------------------------" << std::endl;

    smt::SmtLibReader smtlib_reader(solver);
    smtlib_reader.parse("../design/idpv-test/c_mul64.smt2");
    auto in_a = smtlib_reader.lookup_symbol("__muldf3(double,double)::a!0@1#1");
    auto in_b = smtlib_reader.lookup_symbol("__muldf3(double,double)::b!0@1#1");
    auto in_ret = smtlib_reader.lookup_symbol("__muldf3(double,double)::$tmp::return_value_muldf3_classical!0@1#2");
    std::cout << in_a -> to_string() << std::endl;
    std::cout << in_b -> to_string() << std::endl;
    std::cout << in_ret -> to_string() << std::endl;

    std::cout << "-----------------------------------------------------" << std::endl;

    SymbolicExecutor executor(sts,solver);
    assignment_type initmul = {};
    auto initial_state = executor.convert(initmul);
    executor.init(initial_state);
    executor.set_input(executor.convert({{"io_multiplicand",32},{"io_multiplier",32}}),{});
    executor.sim_one_step();
    auto s1 = executor.get_curr_state();
    std::cout << s1.print();
    auto expr = s1.get_sv().at(sts.lookup("res"));
    std::cout << expr ->to_string() << std::endl;



    return 0;
}