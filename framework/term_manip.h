#pragma once
// #include "assert.h"
// #include <string>
// #include <iomanip>
// #include <unordered_map>

#include "smt-switch/boolector_factory.h"
// #include "/data/wenjifang/wasim-cpp/deps/smt-switch/include/boolector_extensions.h"

#include "smt-switch/smt.h"
#include "smt-switch/smtlib_reader.h"
// #include "/data/wenjifang/wasim-cpp/deps/smt-switch/include/generic_sort.h"

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
smt::TermVec args(const smt::Term & term);

// DFS of a term (arg), free var --> symbol
smt::UnorderedTermSet get_free_variables(const smt::Term & term);

smt::Term free_make_symbol(const std::string & n, smt::Sort symb_sort, std::unordered_map<std::string, int>& name_cnt, smt::SmtSolver& solver);

class PropertyInterface : public smt::SmtLibReader
{
 public:
    PropertyInterface(std::string filename, TransitionSystem & ts);

    typedef SmtLibReader super;

    smt::Term AddAssertions(const smt::Term &in) const;

    void AddAssumptionsToTS();

 protected:
    // overloaded function, used when arg list of function is parsed
    // NOTE: | |  pipe quotes are removed.
    virtual smt::Term register_arg(const std::string & name, const smt::Sort & sort) override;

    std::string filename_;

    TransitionSystem & ts_;

    smt::TermVec assertions_;
    smt::TermVec assumptions_;

};
}