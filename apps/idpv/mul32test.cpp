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
    // executor.set_input(executor.convert({{"A","a"},{"B","b"}}),{});
    auto initial_input = executor.convert({{"A","a"},{"B","b"},{"enable",1},{"reset",0}});

    auto v_a = initial_input.at(sts.lookup("A"));
    auto v_b = initial_input.at(sts.lookup("B"));
    std::cout <<"Vlg a expr: " << v_a ->to_string() << std::endl;
    std::cout <<"Vlg b expr: " << v_b ->to_string() << std::endl;

    executor.set_input(initial_input,{});
    executor.sim_one_step();

    executor.set_input(executor.convert({{"enable",1}}),{});
    executor.sim_one_step();
    auto s1 = executor.get_curr_state();
    std::cout<<s1.print()<<std::endl; 
    // auto v_a = s1.get_sv().at(sts.lookup("regA"));
    // auto v_b = s1.get_sv().at(sts.lookup("regB"));
    auto v_ret = s1.get_sv().at(sts.lookup("result"));
    std::cout <<"Vlg expr: " << v_ret ->to_string() << std::endl;

    // std::cout << "---------------------------C++ smtlib2---------------------------" << std::endl;

    smt::SmtLibReader smtlib_reader(solver);
    smtlib_reader.parse("../design/idpv-test/concat_error.smt2");
    auto c_a = smtlib_reader.lookup_symbol("__mulsf3::a!0@1#1");
    auto c_b = smtlib_reader.lookup_symbol("__mulsf3::b!0@1#1");
    auto c_ret = smtlib_reader.lookup_symbol("__mulsf3::$tmp::return_value_mulsf3_classical!0@1#2");

    std::cout << "C a: " << c_a->to_string() << std::endl;
    std::cout << "C b: " << c_b->to_string() << std::endl;
    std::cout << "C expr: " << c_ret->to_string() << std::endl;

    // std::cout << "------------------------equvi check------------------------------" << std::endl;

    auto check_a = solver->make_term(smt::Equal, v_a, c_a);
    solver->assert_formula(check_a);
    auto r = solver->check_sat();
    if(r.is_sat()){
        std::cout<<"a equal"<<std::endl;
    }

    auto check_b = solver->make_term(smt::Equal, v_b, c_b);
    solver->assert_formula(check_b);
    auto r1 = solver->check_sat();
    if(r1.is_sat()){
        std::cout<<"b equal"<<std::endl;
    }

    // auto ret_extended = solver->make_term(smt::Op(smt::PrimOp::Sign_Extend, 32), expr);
    auto c_ret_32 = solver-> make_term(smt::Op(smt::PrimOp::Extract,31,0),c_ret);
    std::cout<<v_ret->get_sort()<<std::endl;
    std::cout<<c_ret_32->get_sort()<<std::endl;

    auto check_ret = solver->make_term(smt::Equal, v_ret, c_ret_32);
    Term not_equal = solver -> make_term(Not, check_ret);
    TermVec assumptions{not_equal};
    auto res_ret = solver->check_sat_assuming(assumptions);
    if(res_ret.is_unsat()){
        std::cout << "always equal" << std::endl;
        
    } else if(res_ret.is_sat()){
        std::cout<<"va: "<<solver->get_value(v_a)<<std::endl;
        std::cout<<"vb: "<<solver->get_value(v_b)<<std::endl;
        std::cout<<"vexpr: "<<solver->get_value(v_ret)<<std::endl;
        std::cout<<"ca: "<<solver->get_value(c_ret_32)<<std::endl;
        std::cout<<"cb: "<<solver->get_value(c_ret_32)<<std::endl;
        std::cout<<"cexpr:"<<solver->get_value(c_ret_32)<<std::endl;


        std::cout << "exist unequal" << std::endl;
    }

    return 0;
}

/*
a
11001110100000111111000000000001
11001110100000111111000000000001
-1106772096.0

b
01001110011111000001100000000001
01001110011111000001100000000001
1057357888.0

output:
11011101100000011110110010100000
11011101100000011110110010011111
v:−1170254205907107800
c:-1170254068468154368.000000
 

#b10111110100000000000000000000000
#b10000000111111111111111111111111
#b01111111111111111111111111111111
−0.25
−2.3509885615147286 x 10^-38
6.805646932770577 x 10^38  obviously ridiculous


va: #b11001110100000111111000000000001
vb: #b01001110011111000001100000000001
vexpr: #b11011101100000011110110010100000
ca: #b11011101100000011110110010011111
cb: #b11011101100000011110110010011111
cexpr:#b11011101100000011110110010011111

---------------------------------------
change concat_error.smt2

va: #b10111110100000000000000000000000
vb: #b10000000111111111111111111111111
vexpr: #b01111111111111111111111111111111
ca: #b11111111111111111111111111111111
cb: #b11111111111111111111111111111111
cexpr:#b11111111111111111111111111111111


*/