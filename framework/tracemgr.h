#pragma once
#include "../deps/smt-switch/local/include/smt-switch/boolector_factory.h"
#include "../deps/smt-switch/local/include/smt-switch/smt.h"
#include "term_manip.h"
#include "ts.h"
#include <variant>
#include <set>

using namespace std;

namespace wasim
{
using type_record = std::variant<smt::TermVec, smt::UnorderedTermSet, smt::Term>;
class TraceManager
{
public:
    TraceManager(TransitionSystem ts, smt::SmtSolver & s){
        this->ts_ = ts;
        this->solver_ = s;
        this->invar_ = ts.inputvars_;
        this->svar_ = ts.statevars_;
        this->Xvar_ = {};

        this->base_var_ = {};
        this->abs_state_ = {};
        this->abs_state_one_step_ = {};
    }

    TransitionSystem ts_;
    smt::SmtSolver solver_;
    smt::UnorderedTermSet invar_;
    smt::UnorderedTermSet svar_;
    smt::UnorderedTermSet Xvar_;
    smt::UnorderedTermSet base_var_;
    std::vector<StateAsmpt> abs_state_;
    std::vector<StateAsmpt> abs_state_one_step_;

    void record_x_var(wasim::type_record var);
    void record_base_var(wasim::type_record var);
    void remove_base_var(smt::Term var);
    bool record_state_w_asmpt(StateAsmpt state, wasim::type_record Xvar);
    bool record_state_w_asmpt3(std::vector<StateAsmpt> new_state_vec,StateAsmpt state, wasim::type_record Xvar);
    bool record_state_w_asmpt_one_step(StateAsmpt state);
    // void _debug_abs_check(smt::Term expr, smt::TermVec assumptions);
    bool abs_eq(StateAsmpt s_abs, StateAsmpt s2);
    bool check_reachable(StateAsmpt s_in);
    bool check_concrete_enough(StateAsmpt s_in, wasim::type_record Xs);
    StateAsmpt abstract(StateAsmpt s);



};
} // namespace wasim