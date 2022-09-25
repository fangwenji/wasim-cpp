#include "traverse_manip.h"

namespace wasim{

smt::TermVec tag2asmpt_c1(std::string flag, SymbolicExecutor & executor, smt::SmtSolver & solver){
    smt::TermVec ret;
    smt::Term ret_term, ret_term2;
    smt::Term lhs, lhs2;
    smt::Term rhs_0 = solver->make_term(0, solver->make_sort(smt::BV, 1));
    smt::Term rhs_1 = solver->make_term(1, solver->make_sort(smt::BV, 1));
    if(flag == "tag0_0"){
        lhs = executor.sv("stage1_go");
        ret_term = solver->make_term(smt::Equal, lhs, rhs_0);    
    }
    else if(flag == "tag0_1"){
        lhs = executor.sv("stage1_go");
        ret_term = solver->make_term(smt::Equal, lhs, rhs_1);
    }
    else if(flag == "tag1_1"){
        lhs = executor.sv("stage1_go");
        ret_term = solver->make_term(smt::Equal, lhs, rhs_0);
        lhs2 = executor.sv("stage2_go");
        ret_term2 = solver->make_term(smt::Equal, lhs2, rhs_0);
    }
    else if(flag == "tag1_2"){
        lhs = executor.sv("stage2_go");
        ret_term = solver->make_term(smt::Equal, lhs, rhs_1);
    }
    else if(flag == "tag2_2"){
        lhs = executor.sv("stage2_go");
        ret_term = solver->make_term(smt::Equal, lhs, rhs_0);
        lhs2 = executor.sv("stage3_go");
        ret_term2 = solver->make_term(smt::Equal, lhs2, rhs_0);
    }
    else if(flag == "tag2_3"){
        lhs = executor.sv("stage3_go");
        ret_term = solver->make_term(smt::Equal, lhs, rhs_1);
    }
    else if(flag == "tag3_3"){
        lhs = executor.sv("stage3_go");
        ret_term = solver->make_term(smt::Equal, lhs, rhs_0);
    }
    else{
        cout << "<ERROR> Wrong tag transition format!" << endl;
        assert(false);
    }


    ret.push_back(ret_term);
    ret.push_back(ret_term2);


    
    return ret;
}

void extend_branch_list(std::vector<std::vector<StateAsmpt>> branch_list, SymbolicExecutor & executor, TransitionSystem & sts, std::vector<std::string> base_sv, std::string flag, smt::SmtSolver & s){
    auto branch_list_old(branch_list);
    branch_list.clear();
    auto executor_temp(executor);
    smt::UnorderedTermSet base_variable;
    for (const auto n : base_sv){
        base_variable.insert(executor_temp.sv(n));
    }
    SymbolicTraverse traverse_temp(sts, executor_temp, s, base_variable);
    // traverse_temp.traverse();9
}


} // namespace wasim
