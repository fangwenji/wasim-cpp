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
    auto initial_input = executor.convert({{"A","aa"},{"B","bb"},{"reset",0},{"enable",1}});
    auto a_v2 = initial_input.at(sts.lookup("A"));
    auto b_v2 = initial_input.at(sts.lookup("B"));
    executor.set_input(initial_input,{});
    executor.sim_one_step();
    executor.set_input(executor.convert({{"enable",1}}),{});
    executor.sim_one_step();
    auto s1 = executor.get_curr_state(); 
    // auto a = s1.get_sv().at(sts.lookup("regA"));
    // auto b = s1.get_sv().at(sts.lookup("regB"));
    auto expr_v2 = s1.get_sv().at(sts.lookup("result"));
    std::cout << expr_v2 ->to_string() << std::endl;

    // std::cout << "---------------------------1 cycle---------------------------" << std::endl;
    TransitionSystem sts2(solver);
    BTOR2Encoder btor_parser1("../design/idpv-test/singlecycle.btor2", sts2);
    auto a_v1 = sts2.lookup("input_a");
    auto b_v1 = sts2.lookup("input_b");
    auto expr_v1 = sts2.lookup("result");

    // std::cout << "------------------------equvi check------------------------------" << std::endl;

    auto check_a = solver->make_term(smt::Equal, a_v2, a_v1);
    solver->assert_formula(check_a);
    auto r = solver->check_sat();
    if(r.is_sat()){
        std::cout<<"a equal"<<std::endl;
    }

    auto check_b = solver->make_term(smt::Equal, b_v2, b_v1);
    solver->assert_formula(check_b);
    auto r1 = solver->check_sat();
    if(r1.is_sat()){
        std::cout<<"b equal"<<std::endl;
    }

    std::cout<<expr_v2->get_sort()<<std::endl;
    std::cout<<expr_v1->get_sort()<<std::endl;

    auto check_ret = solver->make_term(smt::Equal, expr_v2, expr_v1);
    Term not_equal = solver->make_term(Not, check_ret);
    TermVec assumptions{not_equal};
    auto res_ret = solver->check_sat_assuming(assumptions);
    if(res_ret.is_unsat()){
        std::cout << "always equal" << std::endl;
        
    } else if(res_ret.is_sat()){
        std::cout<<solver->get_value(a_v2)<<std::endl;
        std::cout<<solver->get_value(a_v1)<<std::endl;
        std::cout<<solver->get_value(b_v2)<<std::endl;
        std::cout<<solver->get_value(b_v1)<<std::endl;
        std::cout<<solver->get_value(expr_v2)<<std::endl;
        std::cout<<solver->get_value(expr_v1)<<std::endl;
        std::cout << "exist unequal" << std::endl;
    }

    return 0;
}

/*
#b11111111111111111000110010000001
#b11111111111111111000110010000001
#b01101111100000000111111010010011
#b01101111100000000111111010010011
2cycle:b10110000000000000100010010011010
1cycle:b11111111100000000100010010011010
    
    1870691987

*/