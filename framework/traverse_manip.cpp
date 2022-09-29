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
        ret.push_back(ret_term);  
    }
    else if(flag == "tag0_1"){
        lhs = executor.sv("stage1_go");
        ret_term = solver->make_term(smt::Equal, lhs, rhs_1);
        ret.push_back(ret_term);
    }
    else if(flag == "tag1_1"){
        lhs = executor.sv("stage1_go");
        ret_term = solver->make_term(smt::Equal, lhs, rhs_0);
        lhs2 = executor.sv("stage2_go");
        ret_term2 = solver->make_term(smt::Equal, lhs2, rhs_0);
        ret.push_back(ret_term);
        ret.push_back(ret_term2);
    }
    else if(flag == "tag1_2"){
        lhs = executor.sv("stage2_go");
        ret_term = solver->make_term(smt::Equal, lhs, rhs_1);
        ret.push_back(ret_term);
    }
    else if(flag == "tag2_2"){
        lhs = executor.sv("stage2_go");
        ret_term = solver->make_term(smt::Equal, lhs, rhs_0);
        lhs2 = executor.sv("stage3_go");
        ret_term2 = solver->make_term(smt::Equal, lhs2, rhs_0);
        ret.push_back(ret_term);
        ret.push_back(ret_term2);
    }
    else if(flag == "tag2_3"){
        lhs = executor.sv("stage3_go");
        ret_term = solver->make_term(smt::Equal, lhs, rhs_1);
        ret.push_back(ret_term);
    }
    else if(flag == "tag3_3"){
        lhs = executor.sv("stage3_go");
        ret_term = solver->make_term(smt::Equal, lhs, rhs_0);
        ret.push_back(ret_term);
    }
    else{
        cout << "<ERROR> Wrong tag transition format!" << endl;
        assert(false);
    }


    



    
    return ret;
}

void extend_branch_init(std::vector<std::vector<StateAsmpt>>& branch_list, SymbolicExecutor& executor, TransitionSystem & sts, std::vector<std::string> base_sv, std::string flag, std::vector<TraverseBranchingNode> order, smt::SmtSolver & solver){
    auto branch_list_old(branch_list);
    branch_list.clear();
    auto executor_temp(executor);
    smt::UnorderedTermSet base_variable;
    for (const auto n : base_sv){
        base_variable.insert(executor_temp.sv(n));
    }
    SymbolicTraverse traverse_temp(sts, executor_temp, solver, base_variable);
    auto assumptions = tag2asmpt_c1(flag, executor_temp, solver);
    traverse_temp.traverse(assumptions, order, {});
    cout << "number of state " << flag << ": 1-> " << traverse_temp.tracemgr_.abs_state_.size() << endl;

    for ( auto& nextstate : traverse_temp.tracemgr_.abs_state_){
        std::vector<StateAsmpt> state_vec_extend;
        state_vec_extend.push_back(nextstate);
        branch_list.push_back(state_vec_extend);
        nextstate.print();
        nextstate.print_assumptions();
    }
    cout << "number of state " << flag << " in total: " << branch_list_old.size() << " --> " << branch_list.size() << endl;
}

void extend_branch_next_phase(std::vector<std::vector<StateAsmpt>>& branch_list, SymbolicExecutor & executor, TransitionSystem & sts, std::vector<std::string> base_sv, std::string flag, std::map<wasim::type_conv, wasim::type_conv> phase_maker, std::vector<TraverseBranchingNode> order, smt::SmtSolver & solver){
    auto branch_list_old(branch_list);
    branch_list.clear();
    for (auto state_list_old : branch_list_old){
        auto state_list (state_list_old);
        auto s = state_list.back();
        auto executor_temp (executor);
        auto d = executor_temp.convert(phase_maker);
        executor_temp.set_current_state(s, d);
        smt::UnorderedTermSet base_variable;
        for (const auto n : base_sv){
            base_variable.insert(executor_temp.sv(n));
        }
        SymbolicTraverse traverse_temp(sts, executor_temp, solver, base_variable);
        auto assumptions = tag2asmpt_c1(flag, executor_temp, solver);
        traverse_temp.traverse_one_step(assumptions, order, {});
        cout << "number of state " << flag << ": 1-> " << traverse_temp.tracemgr_.abs_state_one_step_.size() << endl;

        for (const auto& nextstate : traverse_temp.tracemgr_.abs_state_one_step_){
            auto state_vec_extend (state_list);
            state_vec_extend.push_back(nextstate);
            branch_list.push_back(state_vec_extend);
        }
    }
    int start_num = branch_list_old.size();
    int end_num = branch_list.size();
    cout << "number of state " << flag << " in total: " << branch_list_old.size() << " --> " << branch_list.size() << endl;
}


void extend_branch_same_phase(std::vector<std::vector<StateAsmpt>>& branch_list, SymbolicExecutor & executor, TransitionSystem & sts, std::vector<std::string> base_sv, std::string flag, std::map<wasim::type_conv, wasim::type_conv> phase_maker, std::vector<TraverseBranchingNode> order, smt::SmtSolver & solver){
    auto branch_list_old(branch_list);
    branch_list.clear();
    for (auto state_list_old : branch_list_old){
        auto state_list (state_list_old);
        auto s = state_list.back();
        auto executor_temp (executor);
        auto d = executor_temp.convert(phase_maker);
        executor_temp.set_current_state(s, d);
        smt::UnorderedTermSet base_variable;
        for (const auto n : base_sv){
            base_variable.insert(executor_temp.sv(n));
        }
        SymbolicTraverse traverse_temp(sts, executor_temp, solver, base_variable);
        auto assumptions = tag2asmpt_c1(flag, executor_temp, solver);
        traverse_temp.traverse(assumptions, order, {});
        cout << "number of state " << flag << ": 1-> " << traverse_temp.tracemgr_.abs_state_.size() << endl;

        for (const auto& nextstate : traverse_temp.tracemgr_.abs_state_){
            auto state_vec_extend (state_list);
            state_vec_extend.push_back(nextstate);
            branch_list.push_back(state_vec_extend);
        }
    }
    int start_num = branch_list_old.size();
    int end_num = branch_list.size();
    cout << "number of state " << flag << " in total: " << branch_list_old.size() << " --> " << branch_list.size() << endl;
}

} // namespace wasim
