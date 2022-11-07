#include "symtraverse.h"

namespace wasim
{
bool TraverseBranchingNode::next(){
    value_++;
    if(value_ == pow(2, v_width_)){
        return false;
    }
    return true;
}

TraverseBranchingNode TraverseBranchingNode::get_node(){
    wasim::type_v iv_vec;
    iv_vec.push_back(make_pair(v_name_, v_width_));
    // TraverseBranchingNode tmp;
    if(branch_on_inputvar_){
        TraverseBranchingNode tmp(iv_vec, {});
        return tmp; 
    }
    else{
        TraverseBranchingNode tmp({}, iv_vec);
        return tmp;
    }
}

std::string TraverseBranchingNode::repr(){
    auto ret_str = v_name_ + " == " + to_string(value_) + " ";
    return ret_str;
}

std::string PerStateStack::repr(){
    // std::vector<std::string> stack_vec_str;
    // for(auto& node : stack_){
    //     stack_vec_str.push_back(node.repr());
    // }
    // std::string stack_str(stack_vec_str.begin(), stack_vec_str.end());
    std::string stack_str;
    for (auto& node : stack_){
        stack_str += node.repr(); 
    }
    std::string suffix;
    if(no_next_choice_){
        suffix = " (END)";
    }
    else{
        suffix = "";
    }

    auto ret_str = "[" + stack_str + "]" +" ptr: " + to_string(ptr_) + suffix;
    return ret_str;
}

bool PerStateStack::has_valid_choice(){
    return not no_next_choice_;
}

std::pair<smt::UnorderedTermMap, smt::TermVec> PerStateStack::get_iv_asmpt(smt::TermVec assumptions){
    smt::UnorderedTermMap iv;
    smt::TermVec asmpt;
    for(auto& branch_node : stack_){
        auto d = simulator_.convert({{branch_node.v_name_, branch_node.value_}});
        if(branch_node.branch_on_inputvar_){
            iv.insert(d.begin(), d.end());
        }
        else{
            std::vector<std::pair<smt::Term, smt::Term>> l;
            for (const auto& d_items : d){
                auto p = make_pair(d_items.first, d_items.second);
                l.push_back(p);
            }
            assert(l.size() == 1);
            auto asmpt_term = solver_->make_term(smt::Equal, l.at(0).first, l.at(0).second);
            asmpt.push_back(asmpt_term);
        }
    }
    asmpt.insert(asmpt.end(), assumptions.begin(), assumptions.end());
    

    auto ret = make_pair(iv, asmpt);
    return ret;
}

bool PerStateStack::next_choice(){
    bool succ = false;
    while (not succ)
    {
        if(stack_.size() == 0){
            no_next_choice_ = true;
            return false;
        }
        succ = stack_.back().next();
        if(not succ){
            ptr_--;
            stack_.pop_back();
        }
    }
    return true;
    
}

bool PerStateStack::deeper_choice(){
    if(ptr_ == branching_point_.size()){
        return false;
    }
    auto branch_node = branching_point_.at(ptr_);
    stack_.push_back(branch_node.get_node());
    ptr_++;
    return true;
}

bool PerStateStack::check_stack(){
    auto count = 0;
    for(const auto& node : stack_){
        if(node.value_ == 1){
            count ++;
        }
    }

    if(count > 1){
        return false;
    }
    else{
        return true;
    }
}

void SymbolicTraverse::traverse_one_step(smt::TermVec assumptions, std::vector<TraverseBranchingNode> branching_point, std::vector<wasim::StateAsmpt> s_init /*={}*/){
    auto state = executor_.get_curr_state(assumptions);
    StateAsmpt state_init (state); // shallow copy?
    auto reachable = tracemgr_.check_reachable(state);
    if(not reachable){
        tracemgr_.abs_state_one_step_.insert(tracemgr_.abs_state_one_step_.end(), s_init.begin(), s_init.end());
        assert(tracemgr_.abs_state_one_step_.size() == 1);
        cout << "not reachable! skip!" << endl;
        cout << "==============================" << endl;
        cout << "Finished!" << endl;
        cout << "Get #state: " << tracemgr_.abs_state_one_step_.size() << endl;

        auto abs_state = tracemgr_.abs_state_one_step_.at(0);
        //TODO : SIMPLIFICATION
        exit(0);
    }
    auto concrete_enough = tracemgr_.check_concrete_enough(state, executor_.get_Xs());
    assert(concrete_enough);
    auto is_new_state = tracemgr_.record_state_w_asmpt(state, executor_.get_Xs());
    assert(is_new_state);

    PerStateStack init_choice(branching_point, executor_, solver_);

    while (init_choice.has_valid_choice())
    {
        
        cout << ">> [" << init_choice.repr() << "]  ";
        auto iv_asmpt_pair = init_choice.get_iv_asmpt(assumptions);
        auto iv = iv_asmpt_pair.first;
        auto asmpt = iv_asmpt_pair.second;
        executor_.set_input(iv, asmpt);
        executor_.sim_one_step();
        auto state = executor_.get_curr_state();
        auto reachable = tracemgr_.check_reachable(state);

        if(not reachable){
            cout << "not reachable." << endl;
            init_choice.next_choice();
            executor_.backtrack();
            executor_.undo_set_input();
            continue;
        }

        auto concrete_enough = tracemgr_.check_concrete_enough(state, executor_.get_Xs());
        if(not concrete_enough){
            cout << "not concrete. Retry with deeper choice." << endl;
            auto succ = init_choice.deeper_choice();
            if(succ){
                executor_.backtrack();
                executor_.undo_set_input();
                continue;
            }
            cout << "<ERROR>: cannot reach a concrete state even if all choices are made. Future work." << endl;
            auto state = executor_.get_curr_state();
            // state.print();
            assert(false);
        }

        auto is_new_state = tracemgr_.record_state_w_asmpt(state, executor_.get_Xs());

        if(is_new_state){
            cout << "new state!" << endl;
            tracemgr_.record_state_w_asmpt_one_step(state);
        }
        else{
            cout << "already exeists." << endl;
        }

        init_choice.next_choice();
        executor_.backtrack();
        executor_.undo_set_input();
    }

    cout << "=============================" << endl;
    cout << "Finish!" << endl;
    cout << "Get #state: " << tracemgr_.abs_state_one_step_.size() << endl;

    // TODO : simplification procedure
    for(auto& abs_state_one_step : tracemgr_.abs_state_one_step_){
        state_simplify_xvar(abs_state_one_step, executor_.get_Xs(), solver_);
        // sygus_simplify(abs_state_one_step, executor_.get_Xs(), solver_);
    }

    

}

void SymbolicTraverse::traverse(smt::TermVec assumptions, std::vector<TraverseBranchingNode> branching_point, std::vector<StateAsmpt> s_init /*={}*/){
    auto state = executor_.get_curr_state(assumptions);
    auto reachable = tracemgr_.check_reachable(state);
    if(not reachable){
        tracemgr_.abs_state_.insert(tracemgr_.abs_state_.end(), s_init.begin(), s_init.end());
        assert(tracemgr_.abs_state_.size() == 1);
        cout << "not reachable! skip!" << endl;
        cout << "==============================" << endl;
        cout << "Finished!" << endl;
        cout << "Get #state: " << tracemgr_.abs_state_.size() << endl;

        auto abs_state = tracemgr_.abs_state_.at(0);
        //TODO : SIMPLIFICATION
        exit(0);
    }
    auto concrete_enough = tracemgr_.check_concrete_enough(state, executor_.get_Xs());
    assert(concrete_enough);
    auto is_new_state = tracemgr_.record_state_w_asmpt(state, executor_.get_Xs());
    assert(is_new_state);

    PerStateStack init_stack(branching_point, executor_, solver_);
    std::vector<PerStateStack> stack_per_state;
    stack_per_state.push_back(init_stack);
    cout << "init stack per state: " << init_stack.repr() << endl;
    cout << "init tracelen: " << executor_.tracelen() << endl;
    auto init_tracelen = executor_.tracelen() - 1;
    new_state_vec_.push_back(state);

    int tree_branch_num = 0;
    int branch_end_flag = 0;
    while (stack_per_state.size() != 0)
    {
        state = executor_.get_curr_state();
        // state.print_assumptions();
        // int i;
        // cin >> i;
        // auto& current_state_stack = stack_per_state.back();
        cout << "Trace: " << executor_.tracelen() - init_tracelen << " Stack: " << stack_per_state.size() << endl;
        cout << ">> [" ;
        for(auto perstack : stack_per_state){
            cout << perstack.repr() << " " ;
        }
        cout << " ] : " ;

        if(not stack_per_state.back().has_valid_choice()){
            cout << " no new choices, back to prev state" << endl;
            stack_per_state.pop_back();
            if(stack_per_state.size() != 0){
                executor_.backtrack();
                executor_.undo_set_input();
                auto state_vec_temp (new_state_vec_);
                vec_of_state_vec_.push_back(state_vec_temp);
                tree_branch_num ++ ;
                new_state_vec_.pop_back();
                branch_end_flag = 1;
            }
            continue;
        }
        auto iv_asmpt = stack_per_state.back().get_iv_asmpt(assumptions);
        auto iv = iv_asmpt.first;
        auto asmpt = iv_asmpt.second;
        executor_.set_input(iv, asmpt);
        executor_.sim_one_step();
        state = executor_.get_curr_state();
        auto& curr_state = state;

        auto reachable = tracemgr_.check_reachable(state);
        if(not reachable){
            cout << " not reachable." << endl;
            stack_per_state.back().next_choice();
            executor_.backtrack();
            executor_.undo_set_input();
            continue;
        }

        auto concrete_enough = tracemgr_.check_concrete_enough(state, executor_.get_Xs());
        if(not concrete_enough){
            cout << "not concrete. Retry with deeper choice." << endl;
            auto succ = stack_per_state.back().deeper_choice();
            if(succ){
                executor_.backtrack();
                executor_.undo_set_input();
                continue;
            }

            auto state = executor_.get_curr_state();
            // state.print();
            for (const auto & sv : state.sv_){
                auto s = sv.first;
                auto v = sv.second;
                if(expr_contians_X(v, executor_.get_Xs())){
                    cout << v << endl;
                }
            }
            cout << "<ERROR>: cannot reach a concrete state even if all choices are made. Future work." << endl;
            assert(false);
        }

        std::vector<wasim::StateAsmpt> state_vec;
        if(branch_end_flag == 1){
            branch_end_flag = 0;
            state_vec = vec_of_state_vec_[tree_branch_num-1];
        }
        else{
            state_vec = new_state_vec_;
        }

        auto is_new_state = tracemgr_.record_state_w_asmpt3(state_vec, state, executor_.get_Xs());

        if(is_new_state){
            cout << "A new state!" << endl;
            new_state_vec_.push_back(curr_state);

            PerStateStack stack(branching_point, executor_, solver_);
            stack_per_state.push_back(stack);
        }
        else{
            cout << " not new state. Go back. Try next." << endl;

            auto test = stack_per_state.back().next_choice();
            executor_.backtrack();
            executor_.undo_set_input();
        }
    }
    
    cout << "=============================" << endl;
    cout << "Finish!" << endl;
    cout << "Get #state: " << tracemgr_.abs_state_.size() << endl;

    // TODO : simplification procedure
    for(auto& abs_state : tracemgr_.abs_state_){
        state_simplify_xvar(abs_state, executor_.get_Xs(), solver_);
        // sygus_simplify(abs_state, executor_.get_Xs(), solver_);
    }
}
} // namespace wasim

