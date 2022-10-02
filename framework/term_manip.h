#pragma once
// #include "assert.h"
// #include <string>
// #include <iomanip>
// #include <unordered_map>
// #include <boost/variant.hpp>

#include "../deps/smt-switch/local/include/smt-switch/boolector_factory.h"
// #include "../deps/smt-switch/local/include/smt-switch/boolector_extensions.h"

#include "../deps/smt-switch/local/include/smt-switch/smt.h"
// #include "../deps/smt-switch/local/include/smt-switch/generic_sort.h"

// #include "../utils/exceptions.h"

#include "ts.h"
#include "symsim.h"

// #include <iostream>
// #include <unordered_map>
// #include <utility>
// #include <functional>
// #include <vector>
// #include <set>
#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <variant>

using namespace std;

namespace wasim
{

//  return the arguments of a term, <left, right>
smt::TermVec args(smt::Term term);

// DFS of a term (arg), free var --> symbol
smt::UnorderedTermSet get_free_variables(smt::Term term);

smt::Term free_make_symbol(std::string n, smt::Sort symb_sort, std::unordered_map<std::string, int>& name_cnt, smt::SmtSolver& solver);
}