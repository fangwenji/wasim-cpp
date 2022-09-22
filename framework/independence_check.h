#pragma once

#include <string>
// #include <iomanip>
// #include <unordered_map>
// #include <boost/variant.hpp>

// #include "../deps/smt-switch/local/include/smt-switch/boolector_factory.h"

#include "../deps/smt-switch/local/include/smt-switch/smt.h"
// #include "../deps/smt-switch/local/include/smt-switch/generic_sort.h"

#include "../utils/exceptions.h"
#include "assert.h"

// #include "ts.h"

// #include <iostream>
// #include <unordered_map>
// #include <utility>
// #include <functional>
// #include <vector>
// #include <set>
// #include <any>
// #include <variant>

using namespace std;

namespace wasim
{
// bool is_bv_constant(smt::Term e);

bool e_is_always_valid(smt::Term e, smt::TermVec assumptions /*={}*/, smt::SmtSolver s); // no test

bool e_is_always_invalid(smt::Term e, smt::TermVec assumptions /*={}*/, smt::SmtSolver s); // no test

bool e_is_independent_of_v(smt::Term e, smt::Term v, smt::TermVec assumptions /*={}*/, smt::SmtSolver s); // pass

smt::Term substitute_simplify(smt::Term e, smt::Term v, smt::TermVec assumptions /*={}*/, smt::SmtSolver s); // not sure

bool is_valid(smt::Term e, smt::SmtSolver s); // no test

} // namespace wasim

