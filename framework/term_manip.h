#pragma once
#include "framework/symsim.h"
#include "framework/ts.h"
#include "smt-switch/boolector_factory.h"
#include "smt-switch/smt.h"
#include "smt-switch/smtlib_reader.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <variant>
#include <vector>
#include "math.h"

using namespace std;

namespace wasim {

// will try to create a variable starting with name n and will add number suffix
// to avoid naming conflicts
smt::Term free_make_symbol(const std::string & n,
                           smt::Sort symb_sort,
                           std::unordered_map<std::string, int> & name_cnt,
                           smt::SmtSolver & solver);

class PropertyInterface : public smt::SmtLibReader
{
 public:
  PropertyInterface(const std::string & filename, smt::SmtSolver & solver);

  typedef SmtLibReader super;

  smt::Term return_defs();

 protected:
  // overloaded function, used when arg list of function is parsed
  // NOTE: | |  pipe quotes are removed.
  virtual smt::Term register_arg(const std::string & name,
                                 const smt::Sort & sort) override;

  std::string filename_;
};

class StateRW
{
 public:
  StateRW(const smt::SmtSolver & s) : solver_(s) { }

  // Read/Write a single state
  void StateWrite(const wasim::StateAsmpt & state,
                  const std::string & outfile_sv,
                  const std::string & outfile_asmpt);
  StateAsmpt StateRead(const std::string & infile_sv, const std::string & infile_asmpt);

  // Read/Write a tree of states
  void StateWriteTree(const std::vector<std::vector<StateAsmpt>> & branch_list,
                      const std::string & out_dir);
  std::vector<std::vector<StateAsmpt>> StateReadTree(const std::string & in_dir,
                                                     int i,
                                                     int j);
 protected:
  // functions only used internally
  void write_single_term(const std::string & outfile, const smt::Term & expr);
  void write_term(const std::string & outfile, const smt::Term & expr);
  void write_string(const std::string & outfile, const std::string & asmpt_interp);
  bool is_bv_const(const smt::Term & expr);
  smt::Term str2bvnum(const std::string & int_num);

 private:
  smt::SmtSolver solver_;
  // smt::SmtSolver solver_asmpt_;
};


/**
 * Create one-hot expression for one_hot_vec, this allows 0 or 1 to be true
 */
smt::TermVec one_hot0(const smt::TermVec & one_hot_vec, smt::SmtSolver & solver);

// check satisfiability, if out != NULL, will update it with the sat model
smt::Result is_sat_res(const smt::TermVec & expr_vec, const smt::SmtSolver & solver, smt::UnorderedTermMap * out = NULL);

bool is_sat_bool(const smt::TermVec & expr_vec, const smt::SmtSolver & solver);
bool is_valid_bool(const smt::Term & expr, const smt::SmtSolver & solver);

std::vector<std::string> sort_model(const smt::UnorderedTermMap & cex);
}  // namespace wasim
