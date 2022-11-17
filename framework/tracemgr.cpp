#include "tracemgr.h"
#include "independence_check.h"

namespace wasim
{

void TraceManager::record_x_var(wasim::type_record var){
    if(std::holds_alternative<smt::TermVec>(var)){
        auto var_vec = std::get<smt::TermVec>(var);
        smt::UnorderedTermSet var_set (var_vec.begin(), var_vec.end());
        Xvar_.insert(var_set.begin(), var_set.end());
    }
    else if(std::holds_alternative<smt::UnorderedTermSet>(var)){
        auto var_set = std::get<smt::UnorderedTermSet>(var);
        Xvar_.insert(var_set.begin(), var_set.end());
    }
    else{
        auto var_term = std::get<smt::Term>(var);
        Xvar_.insert(var_term);
    }
}

void TraceManager::record_base_var(wasim::type_record var){
    if(std::holds_alternative<smt::TermVec>(var)){
        auto var_vec = std::get<smt::TermVec>(var);
        smt::UnorderedTermSet var_set (var_vec.begin(), var_vec.end());
        base_var_.insert(var_set.begin(), var_set.end());
    }
    else if(std::holds_alternative<smt::UnorderedTermSet>(var)){
        auto var_set = std::get<smt::UnorderedTermSet>(var);
        base_var_.insert(var_set.begin(), var_set.end());
    }
    else{
        auto var_term = std::get<smt::Term>(var);
        base_var_.insert(var_term);
    }
}

void TraceManager::remove_base_var(smt::Term var){
    base_var_.erase(var);
}

bool TraceManager::record_state_w_asmpt(StateAsmpt state, wasim::type_record Xvar){
    record_x_var(Xvar);

    for(auto s : abs_state_){
        if(abs_eq(s, state)){
            return false;
        }
    }
    abs_state_.push_back(abstract(state));
    return true;
}

bool TraceManager::record_state_w_asmpt3(std::vector<StateAsmpt> new_state_vec,StateAsmpt state, wasim::type_record Xvar){
    record_x_var(Xvar);

    for(auto s : new_state_vec){
        if(abs_eq(abstract(s), state)){
            return false;
        }
    }
    abs_state_.push_back(abstract(state));
    return true;
}

bool TraceManager::record_state_w_asmpt_one_step(StateAsmpt state){
    abs_state_one_step_.push_back(abstract(state));
    return true;
}

bool TraceManager::abs_eq(StateAsmpt s_abs, StateAsmpt s2){
    smt::TermVec expr_vec;
    for (const auto& sv : s_abs.sv_){
        if(s2.sv_.find(sv.first) == s2.sv_.end()){
            return false;
        }
        auto v2 = s2.sv_.at(sv.first);
        
        auto expr_tmp = solver_->make_term(smt::Equal, sv.second, v2); //TODO: need test, expr no initialization
        expr_vec.push_back(expr_tmp);
    }

    smt::Term expr;
    if(expr_vec.size()>1){
        expr = solver_->make_term(smt::And, expr_vec);
    }
    else{
        expr = expr_vec.back();
    }
    
    smt::TermVec assumptions;
    assumptions.insert(assumptions.end(), s_abs.asmpt_.begin(), s_abs.asmpt_.end());
    assumptions.insert(assumptions.end(), s2.asmpt_.begin(), s2.asmpt_.end());

    // auto r = solver_->check_sat_assuming(assumptions);
    auto r = is_sat_res(assumptions, solver_);
    if(not(r.is_sat())){
        return false;
    }
    auto valid = e_is_always_valid(expr, assumptions, solver_);
    return valid;

    
}

bool TraceManager::check_reachable(StateAsmpt s_in){
    auto asmpt = s_in.asmpt_;
    // auto r = solver_->check_sat_assuming(asmpt);
    auto r = is_sat_res(asmpt, solver_);
    auto current_asmpt_sat = r.is_sat();
    return current_asmpt_sat;
}

bool TraceManager::check_concrete_enough(StateAsmpt s_in, wasim::type_record Xs){
    record_x_var(Xs);

    if(base_var_.size() == 0){
        cout << "WARNING: set base_var first!" << endl;
        assert(false);
    }

    for(const auto& sv : s_in.sv_){
        auto s = sv.first;
        auto v = sv.second;
        if(base_var_.find(s) == base_var_.end()){
            continue;
        }
        smt::UnorderedTermSet allv_in_v = get_free_variables(v);
        smt::UnorderedTermSet intersec_res;
        // std::set_intersection(allv_in_v.begin(), allv_in_v.end(), Xvar_.begin(), Xvar_.end(), inserter(intersec_res, intersec_res.begin()));
        
        for(const auto var : allv_in_v){
            if(Xvar_.find(var) != Xvar_.end()){
                intersec_res.insert(var);
                // cout << var->to_string() << endl;
            }
        }

        // allv_in_v.insert(Xvar_.begin(), Xvar_.end());

        for (const auto& X : intersec_res){
            auto ind = e_is_independent_of_v(v, X, s_in.asmpt_);
            if(not ind){
                return false;
            }
        }
    }
    return true;
}

StateAsmpt TraceManager::abstract(StateAsmpt s){
    if(base_var_.size() == 0){
        cout << "WARNING: set base_var first!" << endl;
    }
    StateAsmpt s2({}, s.asmpt_, s.assumption_interp_);
    for(const auto& sv : s.sv_){
        auto s = sv.first;
        auto v = sv.second;
        if(base_var_.find(s) != base_var_.end()){
            s2.sv_[s] = v;
        }
    }
    return s2;
}

} // namespace wasim
