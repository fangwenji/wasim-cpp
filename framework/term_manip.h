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
#include <fstream>
#include <regex>
#include "math.h"


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
    PropertyInterface(std::string filename, smt::SmtSolver &solver);

    typedef SmtLibReader super;

    smt::Term return_defs();

 protected:
    // overloaded function, used when arg list of function is parsed
    // NOTE: | |  pipe quotes are removed.
    virtual smt::Term register_arg(const std::string & name, const smt::Sort & sort) override;

    

    std::string filename_;
    smt::SmtSolver & solver_;

};

class StateRW{
public:
   StateRW(smt::SmtSolver & s){
      this->solver_ = s;
   }

   void StateWrite(wasim::StateAsmpt state, std::string outfile_sv, std::string outfile_asmpt);
   StateAsmpt StateRead(std::string infile_sv, std::string infile_asmpt);
   void write_single_term(std::string outfile, smt::Term expr);
   void write_term(std::string outfile, smt::Term expr);
   void write_string(std::string outfile, std::string asmpt_interp);
   bool is_bv_const(smt::Term expr);
   smt::Term str2bvnum(std::string int_num);
   void StateWriteTree(std::vector<std::vector<StateAsmpt>> branch_list, std::string out_dir);
   std::vector<std::vector<StateAsmpt>> StateReadTree(std::string in_dir, int i, int j);

private:
   smt::SmtSolver solver_;
   
};

void getFileNames(string path, vector<string>& files);

smt::Term TermTransfer(smt::Term expr, smt::SmtSolver& solver_old, smt::SmtSolver& solver_new);
StateAsmpt StateTransfer(wasim::StateAsmpt state, smt::SmtSolver& solver_old, smt::SmtSolver& solver_new);
smt::UnorderedTermSet SetTransfer(smt::UnorderedTermSet expr_set, smt::SmtSolver& solver_old, smt::SmtSolver& solver_new);
}