#pragma once
#include "assert.h"
#include "math.h"
// #include <string>
// #include <iomanip>
// #include <unordered_map>

#include "smt-switch/boolector_factory.h"
#include "smt-switch/cvc5_factory.h"
// #include "../deps/smt-switch/local/include/smt-switch/boolector_extensions.h"

#include "smt-switch/smt.h"
#include "term_manip.h"
// #include "../deps/smt-switch/local/include/smt-switch/generic_sort.h"

#include "utils/exceptions.h"

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

#include <iostream>
#include <chrono>
#include <string>
#include <fstream>
#include <time.h>

#include <boost/process.hpp>
#include <boost/process/async.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>


using namespace std;

namespace wasim
{
namespace bp = boost::process;
class Process {

  public:
    Process(std::string &cmd, const int timeout);
    void run();

  private:
    void timeout_handler(boost::system::error_code ec);

    const std::string command;
    const int timeout;

    bool killed = false;
    bool stopped = false;

    std::string stdOut;
    std::string stdErr;
    int returnStatus = 0;

    boost::asio::io_service ios;
    boost::process::group group;
    boost::asio::deadline_timer deadline_timer;
};


void run_cmd(std::string cmd_string, int timeout);

using parsed_info = std::tuple<smt::UnorderedTermSet, smt::UnorderedTermSet, smt::Term, smt::Term, std::string>;

bool expr_contians_X(smt::Term expr, smt::UnorderedTermSet set_of_xvar);

std::string GetTimeStamp();

parsed_info parse_state(StateAsmpt state, smt::Term v, smt::SmtSolver& solver);

smt::Term run_sygus(parsed_info info, smt::UnorderedTermSet set_of_xvar, smt::SmtSolver& solver);

smt::TermVec child_vec_simplify(smt::TermVec child_vec, StateAsmpt state, smt::UnorderedTermSet set_of_xvar, smt::SmtSolver& solver, smt::SmtSolver& solver_cvc5);

smt::Term structure_simplify(smt::Term v, StateAsmpt state, smt::UnorderedTermSet set_of_xvar, smt::SmtSolver& solver, smt::SmtSolver& solver_cvc5);

void sygus_simplify(StateAsmpt& state_btor, smt::UnorderedTermSet set_of_xvar_btor, smt::SmtSolver& solver);

}