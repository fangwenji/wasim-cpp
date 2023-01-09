#pragma once

#include "tracemgr.h"
#include "ts.h"


using namespace std;

namespace wasim {

// attempt to simplify the variable assigments in state_btor
// assuming all expressions in state_btor are in solver btor
// will create a CVC solver inside and do the translation
void sygus_simplify(StateAsmpt & state_btor,
                    const smt::UnorderedTermSet & set_of_xvar_btor,
                    smt::SmtSolver & solver);

}  // namespace wasim