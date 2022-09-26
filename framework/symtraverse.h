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
#include "sygus_simplify.h"

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

using type_v = std::vector<std::pair<std::string, int>>;

class TraverseBranchingNode
{
public:
    TraverseBranchingNode(wasim::type_v input_v /*={}*/, wasim::type_v signal_v /*={}*/){
        assert((input_v.size() <= 1) and (signal_v.size() <= 1));
        assert((input_v.size() == 0) or (signal_v.size() == 0));
        assert(not((input_v.size() == 0) and (signal_v.size() == 0)));
        wasim::type_v iv_vec;
        if(input_v.size() == 1){
            iv_vec.assign(input_v.begin(), input_v.end());
        }
        else if(signal_v.size() == 1){
            iv_vec.assign(signal_v.begin(), signal_v.end());
        }
        else{
            cout << "ERROR: wrong input type!" << endl;
            assert(false);
        }

        if(input_v.size() == 1){
            this->branch_on_inputvar_ = true;
        }
        else{
            this->branch_on_inputvar_ = false;
        }
        this->v_name_ = iv_vec.back().first;
        this->v_width_ = iv_vec.back().second;
        this->v_width_ = 0;
    }

    bool branch_on_inputvar_;
    std::string v_name_;
    int v_width_;
    int value_;

    bool next();
    TraverseBranchingNode get_node();
    std::string repr();
};

class PerStateStack
{
public:
    PerStateStack(std::vector<TraverseBranchingNode> branching_point, SymbolicExecutor & executor, smt::SmtSolver & s) : simulator_(executor)
    {
        this->stack_ = {};
        this->ptr_ = 0;
        this->branching_point_ = branching_point;
        this->no_next_choice_ = false;
        this->solver_ = s;
        // this->simulator_ = executor;
    }

    // PerStateStack(std::vector<TraverseBranchingNode> branching_point, smt::SmtSolver & s, SymbolicExecutor& simulator)
    // {   
    //     this->solver_ = s;
    //     this->stack_ = {};
    //     this->ptr_ = 0;
    //     this->branching_point_ = branching_point;
    //     this->no_next_choice_ = false;
    //     this->simulator_ = simulator;
    // }


    smt::SmtSolver solver_;
    std::vector<TraverseBranchingNode> stack_;
    int ptr_;
    std::vector<TraverseBranchingNode> branching_point_;
    bool no_next_choice_;
    SymbolicExecutor simulator_;

    std::string repr();
    bool has_valid_choice();
    std::pair<smt::UnorderedTermMap, smt::TermVec> get_iv_asmpt(smt::TermVec assumptions);
    bool next_choice();
    bool deeper_choice();
    bool check_stack();

};

class SymbolicTraverse
{
public:
    SymbolicTraverse(TransitionSystem & ts, SymbolicExecutor & executor, smt::SmtSolver & s, smt::UnorderedTermSet base_variable) :executor_(executor), tracemgr_(ts, s)
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

    void traverse_one_step(smt::TermVec assumptions, std::vector<TraverseBranchingNode> branching_point, StateAsmpt s_init);
    void traverse(smt::TermVec assumptions, std::vector<TraverseBranchingNode> branching_point, std::vector<StateAsmpt> s_init);

};

} // namespace wasim
