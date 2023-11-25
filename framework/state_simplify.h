#include "term_manip.h"
#include "ts.h"

using namespace std;

namespace wasim {


// will update xvar_sub with each xvar in (set_of_xvar AND free_var)
// such that it is fixed to 0/1
void get_xvar_sub(const smt::TermVec & assumptions,
                  const smt::UnorderedTermSet & set_of_xvar,
                  const smt::UnorderedTermSet & free_var,
                  const smt::SmtSolver & solver,
                  smt::UnorderedTermMap & xvar_sub);

// check if expr is a constant under assumptions
smt::Term check_if_constant(
    const smt::Term & expr,
    const smt::TermVec & assumptions, 
    const smt::SmtSolver & solver);

// decide if expr (bool) == 0 or 1 under the assumptions
// return 0/1/2 (unknown)
int is_reducible_bool(const smt::Term & expr,
                      const smt::TermVec & assumptions,
                      const smt::SmtSolver & solver);

// decide if expr (bv1) == 0 or 1 under the assumptions
// return 0/1/2 (unknown)
int is_reducible_bv_width1(const smt::Term & expr,
                           const smt::TermVec & assumptions,
                           const smt::SmtSolver & solver);

// try to simplify the conditions in the ITEs of expr
smt::Term expr_simplify_ite(const smt::Term & expr,
                            const smt::TermVec & assumptions,
                            const smt::SmtSolver & solver);

// this function will try some heuristics to simplify state update functions in s
void state_simplify_xvar(StateAsmpt & s,
                         const smt::UnorderedTermSet & set_of_xvar,
                         const smt::SmtSolver & solver);
}  // namespace wasim