#include "state_simplify.h"

namespace wasim{

using namespace smt;

smt::UnorderedTermMap get_xvar_sub(smt::TermVec assumptions, smt::UnorderedTermSet set_of_xvar, smt::UnorderedTermSet free_var, smt::SmtSolver& solver){
    smt::UnorderedTermMap xvar_sub = {};
    smt::TermVec bv1_vec = {};
    auto value_sort = solver->make_sort(smt::BV, 1);
    for(const auto& xvar:set_of_xvar){
        auto xvar_width = xvar->get_sort()->get_width();
        if ((xvar_width == 1) and (free_var.find(xvar) != free_var.end())){
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

bool is_reducible_bool(smt::Term expr, smt::TermVec assumptions, smt::SmtSolver& solver){
    smt::TermVec check_vec_true (assumptions);
    auto bool_sort = solver->make_sort(smt::BOOL);
    smt::Term eq_expr_true = solver->make_term(smt::Equal, expr, solver->make_term(1, bool_sort));
    check_vec_true.push_back(eq_expr_true);
    auto r_t = solver->check_sat_assuming(check_vec_true);

    smt::TermVec check_vec_false (assumptions);
    smt::Term eq_expr_false = solver->make_term(smt::Equal, expr, solver->make_term(0, bool_sort));
    check_vec_false.push_back(eq_expr_false);
    auto r_f = solver->check_sat_assuming(check_vec_false);
    if(not r_t.is_sat()){
        return false;
    }
    if(not r_f.is_sat()){
        return true;
    }
    return NULL; 
}

bool is_reducible_bv_width1(smt::Term expr, smt::TermVec assumptions, smt::SmtSolver& solver){
    smt::TermVec check_vec_true (assumptions);
    auto bv_sort = solver->make_sort(smt::BV, 1);
    smt::Term eq_expr_true = solver->make_term(smt::Equal, expr, solver->make_term(1, bv_sort));
    check_vec_true.push_back(eq_expr_true);
    auto r_t = solver->check_sat_assuming(check_vec_true);

    smt::TermVec check_vec_false (assumptions);
    smt::Term eq_expr_false = solver->make_term(smt::Equal, expr, solver->make_term(0, bv_sort));
    check_vec_false.push_back(eq_expr_false);
    auto r_f = solver->check_sat_assuming(check_vec_false);
    if(not r_t.is_sat()){
        return false;
    }
    if(not r_f.is_sat()){
        return true;
    }
    return NULL; 
}

smt::Term expr_simplify_ite_new(smt::Term expr, smt::TermVec assumptions, smt::SmtSolver& solver){
    smt::UnorderedTermSet cond_set;
    std::queue<smt::Term> que;
    que.push(expr);
    auto bool_sort = solver->make_sort(smt::BOOL);
    auto T = solver->make_term(1, bool_sort);
    auto F = solver->make_term(0, bool_sort);
    smt::UnorderedTermMap subst_map = {};
    while (que.size() != 0)
    {
        auto node = que.front();
        que.pop();
        if(node->get_op() == smt::Ite){
            auto childern = args(node);
            auto cond = childern.at(0);
            if(cond_set.find(cond) == cond_set.end()){
                auto reducible = is_reducible_bool(cond, assumptions, solver);
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
    for(auto sub:subst_map){
        auto f = sub.first;
        auto s = sub.second;
        cout << "f: " << f << endl;
        cout << "f_sort: " << f->get_sort()->to_string() << endl;
        cout << "f_width: " << f->get_sort()->get_width() << endl;
        cout << "s: " << s << endl;
        cout << "s_sort: " << s->get_sort()->to_string() << endl;
        cout << "s_width: " << s->get_sort()->get_width() << endl;
        
    }
    
    auto expr_sub = solver->substitute(expr, subst_map);
    return expr_sub;
}

TermVec remove_ites_under_model(const SmtSolver & solver, const TermVec & terms)
{
  UnorderedTermSet visited;
  UnorderedTermMap cache;

  TermVec to_visit = terms;
  Term solver_true = solver->make_term(true);
  Term t;
  while (to_visit.size()) {
    t = to_visit.back();
    to_visit.pop_back();

    if (visited.find(t) == visited.end()) {
      to_visit.push_back(t);
      visited.insert(t);
      for (const auto & tt : t) {
        to_visit.push_back(tt);
      }
    } else {
      // post-order case

      TermVec cached_children;
      for (const auto & tt : t) {
        cached_children.push_back(tt);
      }
      Op op = t->get_op();

      if (op == Ite) {
        auto r = solver->check_sat_assuming({cached_children[0]});
        // cout << "r: " << cached_children[0] << " -->-- " << r.is_sat() << endl;
        // cout << cached_children[0] << endl;
        if (solver->get_value(cached_children[0]) == solver_true) {
          // if case
          cache[t] = cached_children[1];
        } else {
          // else case
          cache[t] = cached_children[2];
        }
      } else if (cached_children.size()) {
        // rebuild to take into account any changes
        if (!op.is_null()) {
          cache[t] = solver->make_term(op, cached_children);
        } else {
          assert(cached_children.size() == 1);  // must be a constant array
          assert(t->get_sort()->get_sort_kind() == ARRAY);
          cache[t] = solver->make_term(cached_children[0], t->get_sort());
        }
      } else {
        // just map to itself in the cache
        // when there's no children
        cache[t] = t;
      }

      assert(cache.find(t) != cache.end());
    }
  }

  TermVec res;
  res.reserve(terms.size());
  for (const auto & tt : terms) {
    res.push_back(cache.at(tt));
  }
  return res;
}

void state_simplify_xvar(StateAsmpt s, smt::UnorderedTermSet set_of_xvar, smt::SmtSolver& solver){
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
        // s.sv_[var] = expr_simplify_ite_new(expr_new, s.asmpt_, solver);
        auto ret = remove_ites_under_model(solver, {expr_new});
        assert(ret.size() == 1);
        s.sv_[var] = ret.at(0);

    }

}


}