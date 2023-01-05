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

// //  return the arguments of a term, <left, right>
// smt::TermVec args(const smt::Term & term);


smt::Term free_make_symbol(const std::string & n,
                           smt::Sort symb_sort,
                           std::unordered_map<std::string, int> & name_cnt,
                           smt::SmtSolver & solver);

class PropertyInterface : public smt::SmtLibReader
{
 public:
  PropertyInterface(std::string filename, smt::SmtSolver & solver);

  typedef SmtLibReader super;

  smt::Term return_defs();

 protected:
  // overloaded function, used when arg list of function is parsed
  // NOTE: | |  pipe quotes are removed.
  virtual smt::Term register_arg(const std::string & name,
                                 const smt::Sort & sort) override;

  std::string filename_;
  smt::SmtSolver & solver_;
};

class StateRW
{
 public:
  StateRW(smt::SmtSolver & s) : solver_(s) { }

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
  bool is_bv_const(cosnt smt::Term & expr);
  smt::Term str2bvnum(const std::string & int_num);

 private:
  smt::SmtSolver solver_;
  // smt::SmtSolver solver_asmpt_;
};

smt::Term TermTransfer(const smt::Term & expr,
                       smt::SmtSolver & solver_old,
                       smt::SmtSolver & solver_new);
StateAsmpt StateTransfer(const wasim::StateAsmpt & state,
                         smt::SmtSolver & solver_old,
                         smt::SmtSolver & solver_new);
smt::UnorderedTermSet SetTransfer(const smt::UnorderedTermSet & expr_set,
                                  smt::SmtSolver & solver_old,
                                  smt::SmtSolver & solver_new);

/**
 * @brief
 *
 * @param one_hot_vec
 * @param solver
 * @return smt::TermVec
 */
smt::TermVec one_hot(const smt::TermVec & one_hot_vec, smt::SmtSolver & solver);

// get the assignment to all variables in expr
smt::UnorderedTermMap get_model(const smt::Term & expr, smt::SmtSolver & solver);
#error Rethink about this. get_invalid_model works no different from get_model
smt::UnorderedTermMap get_invalid_model(const smt::Term & expr,
                                        smt::SmtSolver & solver);

smt::Result is_sat_res(const smt::TermVec & expr_vec, smt::SmtSolver & solver);
bool is_sat_bool(const smt::TermVec & expr_vec, smt::SmtSolver & solver);
bool is_valid_bool(const smt::Term & expr, smt::SmtSolver & solver);

std::vector<std::string> sort_model(const smt::UnorderedTermMap & cex);
}  // namespace wasim