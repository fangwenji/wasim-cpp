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
    BTOR2Encoder btor_parser("../design/idpv-test/2cyclemul.btor2", sts);
    std::cout << sts.trans()->to_string() << std::endl;

    SymbolicExecutor executor(sts,solver);
    assignment_type initmul = {};
    auto initial_state = executor.convert(initmul);
    executor.init(initial_state);
    executor.set_input(executor.convert({{"A","a"},{"B","b"}}),{});
    executor.sim_one_step();
    executor.set_input(executor.convert({{"enable",1}}),{});
    executor.sim_one_step();
    auto s1 = executor.get_curr_state();
    std::cout<<s1.print()<<std::endl; 
    auto v_a = s1.get_sv().at(sts.lookup("regA"));
    auto v_b = s1.get_sv().at(sts.lookup("regB"));
    auto v_ret = s1.get_sv().at(sts.lookup("result"));
    std::cout << v_ret ->to_string() << std::endl;

    // std::cout << "---------------------------C++ smtlib2---------------------------" << std::endl;

    smt::SmtLibReader smtlib_reader(solver);
    smtlib_reader.parse("../design/idpv-test/concat_error.smt2");
    auto c_a = smtlib_reader.lookup_symbol("__mulsf3::a!0@1#1");
    auto c_b = smtlib_reader.lookup_symbol("__mulsf3::b!0@1#1");
    auto c_ret = smtlib_reader.lookup_symbol("__mulsf3::$tmp::return_value_mulsf3_classical!0@1#2");

    // std::cout << "------------------------equvi check------------------------------" << std::endl;

    auto check_a = solver->make_term(smt::Equal, v_a, c_a);
    solver->assert_formula(check_a);
    auto r = solver->check_sat();
    if(r.is_sat()){
        std::cout<<"111"<<std::endl;
    }

    auto check_b = solver->make_term(smt::Equal, v_b, c_b);
    solver->assert_formula(check_b);
    auto r1 = solver->check_sat();
    if(r1.is_sat()){
        std::cout<<"111"<<std::endl;
    }

    // auto ret_extended = solver->make_term(smt::Op(smt::PrimOp::Sign_Extend, 32), expr);
    auto c_ret_32 = solver-> make_term(smt::Op(smt::PrimOp::Extract,31,0),c_ret);
    std::cout<<v_ret->get_sort()<<std::endl;
    // std::cout<<ret_extended->get_sort()<<std::endl;
    std::cout<<c_ret_32->get_sort()<<std::endl;

    auto check_ret = solver->make_term(smt::Equal, v_ret, c_ret_32);
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