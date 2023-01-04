#include "state_simplify.h"

#include "smt-switch/utils.h"

namespace wasim {

using namespace smt;

void get_xvar_sub(const smt::TermVec & assumptions,
                  const smt::UnorderedTermSet & set_of_xvar,
                  const smt::UnorderedTermSet & free_var,
                  const smt::SmtSolver & solver,
                  smt::UnorderedTermMap & xvar_sub)
{
  auto bv1_sort = solver->make_sort(smt::BV, 1);
  for (const auto & xvar : set_of_xvar) {
    if (free_var.find(xvar) == free_var.end())
      continue;
    if (xvar->get_sort()->get_sort_kind() == smt::SortKind::BOOL) {
      auto reducible = is_reducible_bool(xvar, assumptions, solver);
      if (reducible == 0) {
        xvar_sub[xvar] = solver->make_term(0);
      } else if (reducible == 1) {
        xvar_sub[xvar] = solver->make_term(1);
      }
    // end of BOOL kind
    } else if (xvar->get_sort()->get_sort_kind() == smt::SortKind::BV) {
      if (xvar->get_sort()->get_width() == 1) {
        auto reducible = is_reducible_bv_width1(xvar, assumptions, solver);
        if (reducible == 0) {
          xvar_sub[xvar] = solver->make_term(0, bv1_sort);
        } else if (reducible == 1) {
          xvar_sub[xvar] = solver->make_term(1, bv1_sort);
        }
      }
    } // end if BV kind
    // will not try simplify in the other cases
  } // end of for each xvar
} // end of get_xvar_sub

int is_reducible_bool(const smt::Term & expr,
                      const smt::TermVec & assumptions,
                      const smt::SmtSolver & solver)
{
  smt::TermVec check_vec_true(assumptions);
  smt::Term eq_expr_true =
      solver->make_term(smt::Equal, expr, solver->make_term(1));
  check_vec_true.push_back(eq_expr_true);

  auto r_t = is_sat_res(check_vec_true, solver);

  smt::TermVec check_vec_false(assumptions);
  smt::Term eq_expr_false =
      solver->make_term(smt::Equal, expr, solver->make_term(0));
  check_vec_false.push_back(eq_expr_false);
  // auto r_f = solver->check_sat_assuming(check_vec_false);
  auto r_f = is_sat_res(check_vec_false, solver);

  if (! r_t.is_sat())
    return 0;
  if (! r_f.is_sat())
    return 1;
  return 2;
}

int is_reducible_bv_width1(const smt::Term & expr,
                           const smt::TermVec & assumptions,
                           const smt::SmtSolver & solver)
{
  smt::TermVec check_vec_true(assumptions);
  auto bv_sort = solver->make_sort(smt::BV, 1);
  smt::Term eq_expr_true =
      solver->make_term(smt::Equal, expr, solver->make_term(1, bv_sort));
  check_vec_true.push_back(eq_expr_true);
  // auto r_t = solver->check_sat_assuming(check_vec_true);
  auto r_t = is_sat_res(check_vec_true, solver);

  smt::TermVec check_vec_false(assumptions);
  smt::Term eq_expr_false =
      solver->make_term(smt::Equal, expr, solver->make_term(0, bv_sort));
  check_vec_false.push_back(eq_expr_false);
  // auto r_f = solver->check_sat_assuming(check_vec_false);
  auto r_f = is_sat_res(check_vec_false, solver);
  if (! r_t.is_sat())
    return 0;
  if (! r_f.is_sat())
    return 1;
  return 2;
} // end of is_reducible_bv_width1

smt::Term expr_simplify_ite(const smt::Term & expr,
                            const smt::TermVec & assumptions,
                            const smt::SmtSolver & solver)
{
  smt::UnorderedTermSet cond_set; // deduplicate (make sure we visit the same condition only once)
  std::queue<smt::Term> que;
  que.push(expr);
  auto T = solver->make_term(1);
  auto F = solver->make_term(0);
  smt::UnorderedTermMap subst_map;
  while (que.size() != 0) {
    auto node = que.front();
    que.pop();
    if (node->get_op() == smt::Ite) {
      auto childern = args(node);
      auto cond = childern.at(0);
      if (cond_set.find(cond) == cond_set.end()) {
        auto reducible = is_reducible_bool(cond, assumptions, solver);
        if (reducible == 0) {
          cond_set.insert(cond);
          subst_map[cond] = F;
          que.push(childern.at(2));
        } else if (reducible == 1) {
          cond_set.insert(cond);
          subst_map[cond] = T;
          que.push(childern.at(1));
        } else {
          for (const auto & c : childern)
            que.push(c);
        } // end else not reducible
      } // end if not cached in cond_set

    } else {
      auto children =  args(node);
      for (const auto & c : children)
        que.push(c);
    }
  } // end of traversal of AST
  return solver->substitute(expr, subst_map);
} // end of expr_simplify_ite

void state_simplify_xvar(StateAsmpt & s,
                         const smt::UnorderedTermSet & set_of_xvar,
                         const smt::SmtSolver & solver)
{
  smt::UnorderedTermSet free_vars;
  for (auto sv : s.sv_) {
    auto expr = sv.second;
    smt::get_free_symbols(expr, free_vars);
  }

  smt::UnorderedTermMap xvar_sub;
  get_xvar_sub(s.asmpt_, set_of_xvar, free_vars, solver, xvar_sub);
  smt::UnorderedTermMap sv_to_replace; // try not to change s.sv_ while traversing

  for (const auto & sv : s.sv_) {
    auto var = sv.first;
    auto expr = sv.second;
    auto expr_new = solver->substitute(expr, xvar_sub);
    auto expr_final = expr_simplify_ite(expr_new, s.asmpt_, solver);
    sv_to_replace.emplace(var, expr_final);
  }
  s.sv_.swap(sv_to_replace); // constant time operation
}

}  // namespace wasim