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

    // std::cout << "---------------------------2cycle---------------------------" << std::endl;
  
    TransitionSystem sts(solver);
    BTOR2Encoder btor_parser("../design/idpv-test/2cyclemul.btor2", sts);
    std::cout << sts.trans()->to_string() << std::endl;

    SymbolicExecutor executor(sts,solver);
    assignment_type initmul = {};
    auto initial_state = executor.convert(initmul);
    executor.init(initial_state);
    executor.set_input(executor.convert({{"A",1},{"B",1}}),{});
    executor.sim_one_step();
    executor.set_input(executor.convert({{"enable",1}}),{});
    executor.sim_one_step();
    auto s1 = executor.get_curr_state(); 
    auto a = s1.get_sv().at(sts.lookup("regA"));
    auto b = s1.get_sv().at(sts.lookup("regB"));
    auto expr = s1.get_sv().at(sts.lookup("result"));
    std::cout << expr ->to_string() << std::endl;

    // std::cout << "---------------------------1 cycle---------------------------" << std::endl;
    TransitionSystem sts2(solver);
    BTOR2Encoder btor_parser1("../design/idpv-test/singlecycle.btor2", sts2);
    auto a_1 = sts2.lookup("input_a");
    auto b_1 = sts2.lookup("input_b");
    auto ret_1 = sts2.lookup("result");

    // std::cout << "------------------------equvi check------------------------------" << std::endl;

    auto check_a = solver->make_term(smt::Equal, a, a_1);
    solver->assert_formula(check_a);
    auto r = solver->check_sat();
    if(r.is_sat()){
        std::cout<<"111"<<std::endl;
    }

    auto check_b = solver->make_term(smt::Equal, b, b_1);
    solver->assert_formula(check_b);
    auto r1 = solver->check_sat();
    if(r1.is_sat()){
        std::cout<<"111"<<std::endl;
    }

    // auto ret_extended = solver->make_term(smt::Op(smt::PrimOp::Sign_Extend, 32), expr);
    std::cout<<expr->get_sort()<<std::endl;
    // std::cout<<ret_extended->get_sort()<<std::endl;
    std::cout<<ret_1->get_sort()<<std::endl;

    auto check_ret = solver->make_term(smt::Equal, expr, ret_1);
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