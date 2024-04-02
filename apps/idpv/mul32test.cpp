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
    BTOR2Encoder btor_parser("../design/idpv-test/newmul32.btor2", sts);
    std::cout << sts.trans()->to_string() << std::endl;

    auto a = sts.lookup("input_a");
    auto b = sts.lookup("input_b");
    auto c = sts.lookup("product");

    auto res_next = sts.state_updates().at(c);

    // SymbolicExecutor executor(sts,solver);
    // assignment_type initmul = {};
    // auto initial_state = executor.convert(initmul);
    // executor.init(initial_state);
    // executor.set_input(executor.convert({{"input_a",1},{"input_b",1}}),{});
    // executor.sim_one_step();
    // auto s1 = executor.get_curr_state();
    // std::cout << s1.print();
    // auto expr = s1.get_sv().at(sts.lookup("res"));
    // std::cout << expr ->to_string() << std::endl;

    std::cout << "---------------------------C++ smtlib2---------------------------" << std::endl;

    smt::SmtLibReader smtlib_reader(solver);
    smtlib_reader.parse("../design/idpv-test/cmul32.smt2");
    auto in_a = smtlib_reader.lookup_symbol("__mulsf3::a!0@1#1");
    auto in_b = smtlib_reader.lookup_symbol("__mulsf3::b!0@1#1");
    auto in_ret = smtlib_reader.lookup_symbol("mulsf3_classical::1::result!0@1#1");
    std::cout << in_a -> to_string() << std::endl;
    std::cout << in_b -> to_string() << std::endl;
    std::cout << in_ret -> to_string() << std::endl;

    std::cout << "------------------------equvi check------------------------------" << std::endl;

    auto check_a = solver->make_term(smt::Equal, a, in_a);
    solver->assert_formula(check_a);
    auto r = solver->check_sat();
    if(r.is_sat()){
        std::cout<<"111"<<std::endl;
    }

    auto check_b = solver->make_term(smt::Equal, b, in_b);
    solver->assert_formula(check_b);
    auto r1 = solver->check_sat();
    if(r1.is_sat()){
        std::cout<<"111"<<std::endl;
    }

    auto check_ret = solver->make_term(smt::Equal, res_next, in_ret);
    Term not_equal = solver -> make_term(Not, check_ret);
    TermVec assumptions{not_equal};
    auto res_ret = solver->check_sat_assuming(assumptions);
    if(res_ret.is_unsat()){
        std::cout << "always equal" << std::endl;
        
    } else if(res_ret.is_sat()){
        std::cout << "exist unequal" << std::endl;
    }



    return 0;
}