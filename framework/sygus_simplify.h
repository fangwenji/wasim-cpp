#pragma once
#include "assert.h"
#include "math.h"
// #include <string>
// #include <iomanip>
// #include <unordered_map>
// #include <boost/variant.hpp>

#include "../deps/smt-switch/local/include/smt-switch/boolector_factory.h"
// #include "../deps/smt-switch/local/include/smt-switch/boolector_extensions.h"

#include "../deps/smt-switch/local/include/smt-switch/smt.h"
#include "term_manip.h"
// #include "../deps/smt-switch/local/include/smt-switch/generic_sort.h"

#include "../utils/exceptions.h"

#include "ts.h"
#include "symsim.h"
#include "tracemgr.h"

// #include <iostream>
// #include <unordered_map>
// #include <utility>
// #include <functional>
// #include <vector>
// #include <set>
// #include <any>
#include <variant>

using namespace std;

namespace wasim
{
bool expr_contians_X(smt::Term expr, smt::UnorderedTermSet set_of_xvar);
}