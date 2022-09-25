#include "sygus_simplify.h"

namespace wasim{

bool expr_contians_X(smt::Term expr, smt::UnorderedTermSet set_of_xvar){
    auto vars_in_expr = get_free_variable(expr);
    for (const auto & var : vars_in_expr){
        if(set_of_xvar.find(var) == set_of_xvar.end()){
            return false;
        }
    }
    return true;
}
}