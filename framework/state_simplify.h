#include "term_manip.h"
#include <queue>
#include "smt-switch/substitution_walker.h"

using namespace std;

namespace wasim
{
smt::UnorderedTermMap get_xvar_sub(smt::TermVec assumptions, smt::UnorderedTermSet set_of_xvar, smt::UnorderedTermSet free_var, smt::SmtSolver& solver);

int is_reducible_bool(smt::Term expr, smt::TermVec assumptions, smt::SmtSolver& solver);

int is_reducible_bv_width1(smt::Term expr, smt::TermVec assumptions, smt::SmtSolver& solver);

smt::Term expr_simplify_ite_new(smt::Term expr, smt::TermVec assumptions,       smt::SmtSolver& solver);

smt::TermVec remove_ites_under_model(const smt::SmtSolver & solver,
                                     const smt::TermVec & terms);

void state_simplify_xvar(StateAsmpt& s, smt::UnorderedTermSet set_of_xvar, smt::SmtSolver& solver);
}