#pragma once
#include "assert.h"
#include <string>
#include <iomanip>
#include <unordered_map>
#include <boost/variant.hpp>

#include "../deps/smt-switch/local/include/smt-switch/boolector_factory.h"
// #include "../deps/smt-switch/local/include/smt-switch/boolector_extensions.h"

#include "../deps/smt-switch/local/include/smt-switch/smt.h"
#include "../deps/smt-switch/local/include/smt-switch/generic_sort.h"

#include "../utils/exceptions.h"

#include "ts.h"
#include "term_manip.h"

#include <iostream>
#include <unordered_map>
#include <utility>
#include <functional>
#include <vector>
#include <set>
#include <any>
#include <variant>

using namespace std;

namespace wasim {
using type_conv = std::variant<int, std::string>;
// class Value
// {
// public:
//     Value();
//     Value(int val);
//     Value(const std::string& val);
//     Value(smt::Term val);

//     bool isString() const;
//     bool isInt() const;

//     const std::string& string() const;
//     uint32_t uint32() const;
//     smt::Term term() const;

// private:
//     boost::variant<int, std::string> _data;
// };

class ChoiceItem
{
public:
    ChoiceItem(smt::TermVec assumptions, smt::UnorderedTermMap var_assign){
        // this->_solver = smt::BoolectorSolverFactory::create(false);
        this->assumptions_ = assumptions;  // TODO: need to add copy
        this->var_assign_ = var_assign;
        this->UsedInSim_ = false;
    }
    void setSim();
    void CheckSimed();
    void record_prev_assumption_len(int l);
    int get_prev_assumption_len();

  // ~StateAsmpt();
// private:
    smt::TermVec assumptions_;
    smt::UnorderedTermMap var_assign_;
    bool UsedInSim_;
    int assmpt_len_;

};

class SymbolicExecutor
{
public:
    SymbolicExecutor(TransitionSystem ts, smt::SmtSolver & s){
        this->ts_ = ts; // transition system
        this->solver_ = s;

        // input variables & state variables
        this->invar_ = ts.inputvars_;
        this->svar_ = ts.statevars_;
        
        this->trace_ = {}; // vector of state var assignment. A var assignment is a unordered_map: v -> value
        this->history_choice_ = {};
        this->history_assumptions_ = {};
        this->history_assumptions_interp_ = {};
        this->name_cnt_ = {};
        this->Xvar_ = {};
    }
    // SymbolicExecutor(TransitionSystem ts){
    //     this->ts_ = ts; // transition system
    //     this->solver_ = smt::BoolectorSolverFactory::create(false);

    //     // input variables & state variables
    //     this->invar_ = ts.inputvars_;
    //     this->svar_ = ts.statevars_;
        
    //     this->trace_ = {}; // vector of state var assignment. A var assignment is a unordered_map: v -> value
    //     this->history_choice_ = {};
    //     this->history_assumptions_ = {};
    //     this->history_assumptions_interp_ = {};
    //     this->name_cnt_ = {};
    //     this->Xvar_ = {};
    // }

    TransitionSystem ts_;
    smt::SmtSolver solver_;
    smt::UnorderedTermSet invar_;
    smt::UnorderedTermSet svar_;
    std::vector<smt::UnorderedTermMap> trace_;
    std::vector<ChoiceItem> history_choice_;
    std::vector<std::vector<smt::Term>> history_assumptions_;
    std::vector<std::vector<std::string>> history_assumptions_interp_;
    std::unordered_map<std::string,int> name_cnt_;
    smt::UnorderedTermSet Xvar_;

    int tracelen();
    auto all_assumptions();
    auto all_assumption_interp();
    smt::Term sv(std::string n);
    smt::Term cur(std::string n);
    void _check_only_invar(smt::UnorderedTermMap vdict);
    bool _expr_only_sv(smt::Term expr);
    // smt::UnorderedTermMap convert(std::map<std::any, std::any> vdict);
    smt::UnorderedTermMap convert(std::map<wasim::type_conv, wasim::type_conv> vdict);
    void backtrack();
    void init(smt::UnorderedTermMap var_assignment = {});
    void set_current_state(StateAsmpt s, smt::UnorderedTermMap d);
    void print_current_step();
    void print_current_step_assumptions();
    void set_input(smt::UnorderedTermMap invar_assign, const smt::TermVec pre_assumptions);
    void undo_set_input();
    std::any interpret_state_expr_on_curr_frame(std::any expr);
    void sim_one_step();
    void sim_one_step_direct();
    smt::UnorderedTermSet get_Xs();
    smt::Term new_var(int bitwdth, std::string vname = "var", bool x = true);
    StateAsmpt get_curr_state(smt::TermVec assumptions = {});
    auto set_var(int bitwdth, std::string vname = "var");



};
}
