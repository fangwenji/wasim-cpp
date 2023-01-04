#include "tracemgr.h"
#include "independence_check.h"

#include "smt-switch/utils.h"

namespace wasim {

bool TraceManager::record_state(const StateAsmpt & state,
                                const smt::UnorderedTermSet & Xvar)
{
  record_x_var(Xvar);

  for (const auto & s : abs_state_) {
    if (abs_eq(s, state)) {
      return false;
    }
  }
  abs_state_.push_back(abstract(state));
  return true;
}

bool TraceManager::record_state_nonexisted_in_vec(
                          const std::vector<StateAsmpt> & new_state_vec,
                          const StateAsmpt & state,
                          const smt::UnorderedTermSet & Xvar)
{
  record_x_var(Xvar);

  for (const auto & s : new_state_vec) {
    if (abs_eq(s, state)) { // abs_eq will ensure it only compare on base_var
      return false;
    }
  }
  abs_state_.push_back(abstract(state));
  return true;
}


bool TraceManager::abs_eq(const StateAsmpt & s_abs, const StateAsmpt & s2)
{
  smt::TermVec expr_vec;
  // for each of the state var in s_abs, find the expression in both
  for (const auto & sv : s_abs.sv_) {
    if (base_var_.find(sv.first) == base_var_.end())
      continue;
    // now sv.first is in base var
    if (s2.sv_.find(sv.first) == s2.sv_.end())
      return false;
    
    auto v2 = s2.sv_.at(sv.first);

    auto expr_tmp = solver_->make_term(
        smt::Equal, sv.second, v2);  // TODO: need test, expr no initialization
    expr_vec.push_back(expr_tmp);
  }

  smt::Term expr;
  if (expr_vec.size() > 1) {
    expr = solver_->make_term(smt::And, expr_vec);
  } else {
    expr = expr_vec.back();
  }

  smt::TermVec assumptions;
  assumptions.insert(
      assumptions.end(), s_abs.asmpt_.begin(), s_abs.asmpt_.end());
  assumptions.insert(assumptions.end(), s2.asmpt_.begin(), s2.asmpt_.end());

  // auto r = solver_->check_sat_assuming(assumptions);
  auto r = is_sat_res(assumptions, solver_);
  if (!r.is_sat()) {
    // add this sanity check
    throw SmtException("[TraceManager::abs_eq] overly constrained!");
    return false;
  }
  auto valid = e_is_always_valid(expr, assumptions, solver_);
  return valid;
}

bool TraceManager::check_reachable(const StateAsmpt & s_in)
{
  const auto & asmpt = s_in.asmpt_;
  // auto r = solver_->check_sat_assuming(asmpt);
  auto r = is_sat_res(asmpt, solver_);
  auto current_asmpt_sat = r.is_sat();
  return current_asmpt_sat;
}

bool TraceManager::check_concrete_enough(const StateAsmpt & s_in, const smt::UnorderedTermSet & Xs)
{
  record_x_var(Xs);

  if (base_var_.size() == 0) {
    cout << "WARNING: set base_var first!" << endl;
    assert(false);
  }

  for (const auto & sv : s_in.sv_) {
    auto s = sv.first;
    auto v = sv.second;
    if (base_var_.find(s) == base_var_.end()) {
      continue;
    }
    smt::UnorderedTermSet allv_in_v;
    smt::UnorderedTermSet intersec_res;

    smt::get_free_symbols(v, allv_in_v);
    for (const auto & var : allv_in_v) {
      if (Xvar_.find(var) != Xvar_.end()) {
        intersec_res.insert(var);
      }
    } // intersec_res is now all X contained in v

    for (const auto & X : intersec_res) {
      auto ind = e_is_independent_of_v(v, X, s_in.asmpt_);
      if (!ind) {
        return false;
      }
    }
  }
  return true;
} // end of check_concrete_enough

StateAsmpt TraceManager::abstract(const StateAsmpt & s)
{
  if (base_var_.size() == 0) {
    cout << "WARNING: set base_var first!" << endl;
  }
  StateAsmpt s2({}, s.asmpt_, s.assumption_interp_);
  for (const auto & sv : s.sv_) {
    auto s = sv.first;
    auto v = sv.second;
    if (base_var_.find(s) != base_var_.end()) {
      s2.sv_.emplace(s, v);
    }
  }
  return s2;
}

}  // namespace wasim
