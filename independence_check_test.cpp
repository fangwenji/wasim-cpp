#include "framework/ts.h"
#include "framework/btor2_encoder.h"
#include "framework/independence_check.h"
#include "symsim.h"
#include "deps/smt-switch/local/include/smt-switch/boolector_factory.h"

#include "assert.h"

using namespace wasim;
using namespace smt;
using namespace std;

int main(){

    smt::SmtSolver s = smt::BoolectorSolverFactory::create(false);
    s->set_opt("incremental", "true");
    s->set_opt("produce-models", "true");
    s->set_opt("produce-unsat-assumptions","true");
    auto bv_type = s->make_sort(smt::BV, 4);
    auto varA = s->make_symbol("A", bv_type);
    auto varB = s->make_symbol("B", bv_type);
    auto f = s->make_term(BVOr, varA, s->make_term(BVAnd, varA, varB));


    cout << "varA: " << varA << endl;
    cout << "varB: " << varB << endl;
    cout << "f: " << f << endl;
    cout << e_is_independent_of_v(f, varA, {}, s) << endl;
    cout << e_is_independent_of_v(f, varB, {}, s) << endl;
    cout << f << endl;

    auto subf = substitute_simplify(f, varB, {}, s);
    

    return 0;
    
}
