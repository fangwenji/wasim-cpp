#include "state_simplify.h"

namespace wasim{

using namespace smt;

smt::UnorderedTermMap get_xvar_sub(smt::TermVec assumptions, smt::UnorderedTermSet set_of_xvar, smt::UnorderedTermSet free_var, smt::SmtSolver& solver){
    smt::UnorderedTermMap xvar_sub = {};
    smt::TermVec bv1_vec = {};
    auto value_sort = solver->make_sort(smt::BV, 1);
    for(const auto& xvar:set_of_xvar){
        auto xvar_width = xvar->get_sort()->get_width();
        if ((xvar_width == 1) && (free_var.find(xvar) != free_var.end())){
            bv1_vec.push_back(xvar);
            auto reducible = is_reducible_bv_width1(xvar, assumptions, solver);
            if(reducible == 0){
                xvar_sub[xvar] = solver->make_term(0, value_sort);
            }
            else if(reducible == 1){
                xvar_sub[xvar] = solver->make_term(1, value_sort);
            }
        }
    }

    return xvar_sub;
}

int is_reducible_bool(smt::Term expr, smt::TermVec assumptions, smt::SmtSolver& solver){
    smt::TermVec check_vec_true (assumptions);
    smt::Term eq_expr_true = solver->make_term(smt::Equal, expr, solver->make_term(1));
    check_vec_true.push_back(eq_expr_true);
    // auto r_t = solver->check_sat_assuming(check_vec_true);
    auto r_t = is_sat_res(check_vec_true, solver);

    

    smt::TermVec check_vec_false (assumptions);
    smt::Term eq_expr_false = solver->make_term(smt::Equal, expr, solver->make_term(0));
    check_vec_false.push_back(eq_expr_false);
    // auto r_f = solver->check_sat_assuming(check_vec_false);
    auto r_f = is_sat_res(check_vec_false, solver);
    if(not r_t.is_sat()){
        return 0;
    }
    if(not r_f.is_sat()){
        return 1;
    }
    return 2; 
}

int is_reducible_bv_width1(smt::Term expr, smt::TermVec assumptions, smt::SmtSolver& solver){
    smt::TermVec check_vec_true (assumptions);
    auto bv_sort = solver->make_sort(smt::BV, 1);
    smt::Term eq_expr_true = solver->make_term(smt::Equal, expr, solver->make_term(1, bv_sort));
    check_vec_true.push_back(eq_expr_true);
    // auto r_t = solver->check_sat_assuming(check_vec_true);
    auto r_t = is_sat_res(check_vec_true, solver);

    smt::TermVec check_vec_false (assumptions);
    smt::Term eq_expr_false = solver->make_term(smt::Equal, expr, solver->make_term(0, bv_sort));
    check_vec_false.push_back(eq_expr_false);
    // auto r_f = solver->check_sat_assuming(check_vec_false);
    auto r_f = is_sat_res(check_vec_false, solver);
    if(not r_t.is_sat()){
        return 0;
    }
    if(not r_f.is_sat()){
        return 1;
    }
    return 2; 
}

smt::Term expr_simplify_ite_new(smt::Term expr, smt::TermVec assumptions, smt::SmtSolver& solver){
    smt::UnorderedTermSet cond_set;
    std::queue<smt::Term> que;
    que.push(expr);
    auto T = solver->make_term(1);
    auto F = solver->make_term(0);
    smt::UnorderedTermMap subst_map = {};
    while (que.size() != 0)
    {
        auto node = que.front();
        que.pop();
        if(node->get_op() == smt::Ite){
            auto childern = args(node);
            auto cond = childern.at(0);
            if(cond_set.find(cond) == cond_set.end()){
                auto reducible = is_reducible_bv_width1(cond, assumptions, solver);
                if(reducible == 0){
                    cond_set.insert(cond);
                    subst_map[cond] = F;
                    que.push(childern.at(2));
                }
                else if(reducible == 1){
                    cond_set.insert(cond);
                    subst_map[cond] = T;
                    que.push(childern.at(1));
                }
                else{
                    for (auto c:childern){
                        que.push(c);
                    }
                }
            }


        }
        else{
            for(auto c:args(node)){
                que.push(c);
            }
        }

    }

    smt::SubstitutionWalker sw(solver, subst_map);
    auto expr_sub = sw.visit(expr);
    return expr_sub;
}


void state_simplify_xvar(StateAsmpt& s, smt::UnorderedTermSet set_of_xvar, smt::SmtSolver& solver){
    smt::TermVec eq_vec = {};
    for (auto sv: s.sv_){
        auto var = sv.first;
        auto expr = sv.second;
        auto eq = solver->make_term(smt::Equal, var, expr);
        eq_vec.push_back(eq);
    }
    auto state_and = solver->make_term(smt::And, eq_vec);
    auto free_var = get_free_variables(state_and);

    smt::UnorderedTermMap usr_sub = {};
    for (auto sv: s.sv_){
        auto var = sv.first;
        auto expr = sv.second;
        auto expr_new = solver->substitute(expr, usr_sub);
        s.sv_[var] = expr_new;
    }

    auto xvar_sub = get_xvar_sub(s.asmpt_, set_of_xvar, free_var, solver);
     for (auto sv: s.sv_){
        auto var = sv.first;
        auto expr = sv.second;
        auto expr_new = solver->substitute(expr, xvar_sub);
        auto expr_final = expr_simplify_ite_new(expr_new, s.asmpt_, solver);
        s.sv_[var] = expr_final;

    }

}


}