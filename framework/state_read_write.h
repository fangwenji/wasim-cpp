#pragma once
#include <framework/ts.h>
#include <framework/symsim.h>

namespace wasim {

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

} // end of namespace wasim
