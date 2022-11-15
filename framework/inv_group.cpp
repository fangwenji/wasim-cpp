#include "inv_group.h"
using namespace std;

namespace wasim
{

smt::Term tobool(smt::Term expr, smt::SmtSolver& solver){
    return solver->make_term(smt::Equal, expr, solver->make_term(1));
}

void InvGroup::branch2state(){

    // 1. get state group
    for(const auto& state_list:branch_list_){
        smt::Term state_expr_single = {};
        smt::TermVec state_expr_vec = {};
        for(const auto& sv:state_list.at(layer_).sv_){
            auto var = sv.first;
            auto value = sv.second;
            state_expr_single = solver_->make_term(smt::Equal, var, value);
            state_expr_vec.push_back(state_expr_single);
        }
        smt::Term state_expr;
        if(state_expr_vec.size()>1){
            state_expr = solver_->make_term(smt::And, state_expr_vec);
        }
        else{
            state_expr = state_expr_vec.at(0);
        }

        state_group_.push_back(state_expr);
    }
    // get state group end

    // 2. get inv group
    for (const auto& state_expr:state_group_){
        auto inv_expr = solver_->make_term(smt::Implies, tag_, state_expr);
        inv_group_.push_back(inv_expr);
    }

    //3. get state dedup
    for (const auto& state_expr:state_group_){
        if(std::find(state_dedup_.begin(), state_dedup_.end(), state_expr) != state_dedup_.end()){
            // found state_expr in state_dedup
            continue;
        }
        else{
            state_dedup_.push_back(state_expr);
        }
    }

    //4. get inv dedup
    for (auto inv_expr:inv_group_){
        if(std::find(inv_dedup_.begin(), inv_dedup_.end(), inv_expr) != inv_dedup_.end()){
            // found state_expr in state_dedup
            continue;
        }
        inv_dedup_.push_back(inv_expr);
    }


}

smt::TermVec InvGroup::state_group(){
    return state_group_;
}

smt::TermVec InvGroup::inv_group(){
    return inv_group_;
}

smt::TermVec InvGroup::state_dedup(){
    return state_dedup_;
}

smt::TermVec InvGroup::inv_dedup(){
    return inv_dedup_;
}

smt::TermVec InvGroup::extract_prop(std::string var_name){
    smt::TermVec extract_vec = {};
    for(const auto& state_list:branch_list_){
        for(const auto& sv:state_list.at(layer_).sv_){
            auto var = sv.first;
            auto value = sv.second;
            if(var->to_string() == var_name){
                extract_vec.push_back(value);
            }
        }
    }
    return extract_vec;
}

}
