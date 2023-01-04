#pragma once
#include "assert.h"
#include "math.h"

#include "smt-switch/boolector_factory.h"

#include "smt-switch/smt.h"
#include "term_manip.h"

#include "utils/exceptions.h"

#include "state_simplify.h"
#include "sygus_simplify.h"
#include "symsim.h"
#include "tracemgr.h"
#include "ts.h"

#include <variant>

using namespace std;

namespace wasim {


class TraverseBranchingNode
{
  protected:
    bool branch_on_inputvar_;
    std::string v_name_;
    int v_width_;
    int value_;
  public:
    using type_v = std::vector<std::pair<std::string, int>>;

    TraverseBranchingNode(type_v input_v,
                          type_v signal_v):
        value_(0)
    {
      assert((input_v.size() <= 1) && (signal_v.size() <= 1));
      assert((input_v.size() == 0) || (signal_v.size() == 0));
      assert(!((input_v.size() == 0) && (signal_v.size() == 0)));
      type_v iv_vec;
      if (input_v.size() == 1) {
        iv_vec.assign(input_v.begin(), input_v.end());
      } else if (signal_v.size() == 1) {
        iv_vec.assign(signal_v.begin(), signal_v.end());
      } else {
        cout << "ERROR: wrong input type!" << endl;
        assert(false);
      }

      if (input_v.size() == 1) {
        branch_on_inputvar_ = true;
      } else {
        branch_on_inputvar_ = false;
      }
      v_name_ = iv_vec.back().first;
      v_width_ = iv_vec.back().second;
    }

  bool next();
  TraverseBranchingNode get_node() const;
  std::string repr() const;
  int value() const {return value_;}
};

class PerStateStack
{
 public:
  PerStateStack(std::vector<TraverseBranchingNode> branching_point,
                SymbolicExecutor & executor,
                smt::SmtSolver & s)
      : simulator_(executor)
  {
    this->stack_ = {};
    this->ptr_ = 0;
    this->branching_point_ = branching_point;
    this->no_next_choice_ = false;
    this->solver_ = s;
    // this->simulator_ = executor;
  }


  smt::SmtSolver solver_;
  std::vector<TraverseBranchingNode> stack_;
  int ptr_;
  std::vector<TraverseBranchingNode> branching_point_;
  bool no_next_choice_;
  SymbolicExecutor simulator_;

  std::string repr() const;
  bool has_valid_choice() const { return !no_next_choice_; }
  std::pair<smt::UnorderedTermMap, smt::TermVec> get_iv_asmpt(
      smt::TermVec assumptions);
  bool next_choice();
  bool deeper_choice();
};

class SymbolicTraverse
{
 public:
  SymbolicTraverse(TransitionSystem & ts,
                   SymbolicExecutor & executor,
                   smt::SmtSolver & s,
                   smt::UnorderedTermSet base_variable)
      : executor_(executor), tracemgr_(ts, s)
  {
    this->sts_ = ts;
    this->base_variable_ = base_variable;
    this->s_concrete_ = {};
    this->new_state_vec_ = {};
    this->vec_of_state_vec_ = {};
    this->tracemgr_.record_base_var(base_variable);
    this->solver_ = s;
  }

  TransitionSystem sts_;
  smt::SmtSolver solver_;
  SymbolicExecutor executor_;
  TraceManager tracemgr_;
  smt::UnorderedTermSet base_variable_;
  smt::UnorderedTermMap s_concrete_;
  std::vector<StateAsmpt> new_state_vec_;
  std::vector<std::vector<StateAsmpt>> vec_of_state_vec_;

  void traverse_one_step(smt::TermVec assumptions,
                         std::vector<TraverseBranchingNode> branching_point,
                         std::vector<wasim::StateAsmpt> s_init /*={}*/);
  void traverse(smt::TermVec assumptions,
                std::vector<TraverseBranchingNode> branching_point,
                std::vector<StateAsmpt> s_init /*={}*/);
};

}  // namespace wasim
