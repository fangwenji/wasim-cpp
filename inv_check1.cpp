#include "framework/ts.h"
#include "framework/btor2_encoder.h"
#include "framework/symsim.h"
#include "smt-switch/boolector_factory.h"
#include "assert.h"
#include "framework/term_manip.h"
#include "framework/symtraverse.h"
#include "framework/term_manip.h"
#include "framework/traverse_manip.h"
#include "config/testpath.h"
#include <chrono>
#include "framework/inv_group.h"

using namespace wasim;
// using namespace smt;
using namespace std;
using namespace chrono;


int main(){
    auto start = system_clock::now();
    std::unordered_set<std::string> base_sv = {"wen_stage1", "wen_stage2", "stage1", "stage2", "stage3"};
    TraverseBranchingNode node0({make_pair("rst", 1)}, {});
    TraverseBranchingNode node1({}, {make_pair("stage1_go", 1)});
    TraverseBranchingNode node2({}, {make_pair("stage2_go", 1)});
    TraverseBranchingNode node3({}, {make_pair("stage3_go", 1)});

    auto order = {node0, node1, node2, node3};

    // This reference the reference path
    std::string input_file =  PROJECT_SOURCE_DIR "/design/testcase1-simple_pipe/simple_pipe.btor2";

    
    smt::SmtSolver solver = smt::BoolectorSolverFactory::create(false);
    solver->set_logic("QF_UFBV");
    solver->set_opt("incremental", "true");
    solver->set_opt("produce-models", "true");
    solver->set_opt("produce-unsat-assumptions", "true");
    TransitionSystem sts(solver);
    BTOR2Encoder btor_parser(input_file, sts);


    SymbolicExecutor executor(sts, solver);

    assignment_type init_map = {
        {"wen_stage1","v1"},
        {"wen_stage2","v2"},
        {"stage1","a"},
        {"stage2","b"},
        {"stage3","c"}
        };
    
    auto map = executor.convert(init_map);
    executor.init(map);
    std::vector<StateAsmpt> state_list;
    std::vector<std::vector<StateAsmpt>> branch_list;    
    StateRW staterw(solver);
    std::string out_dir = PROJECT_SOURCE_DIR "/output/c1/";
    branch_list = staterw.StateReadTree(out_dir, 345, 7);

  
    auto tag0 = tobool(executor.var("tag0"), solver);
    auto tag1 = tobool(executor.var("tag1"), solver);
    auto tag2 = tobool(executor.var("tag2"), solver);
    auto tag3 = tobool(executor.var("tag3"), solver);
    auto wen_stage1 = executor.var("wen_stage1");
    auto wen_stage2 = executor.var("wen_stage2");
    auto stage1 = executor.var("stage1");
    auto stage2 = executor.var("stage2");
    auto stage3 = executor.var("stage3");
    auto reg_v = executor.var("reg_v");
    auto reg_v_comp = executor.var("reg_v_comp");
    auto rst = executor.var("rst");

    auto state_init = branch_list.at(0).at(0);
    // auto asmpt_init = solver->make_term(smt::And, state_init.asmpt_);
    smt::TermVec state_expr_vec = {};
    for(const auto& sv:state_init.sv_){
        state_expr_vec.push_back(solver->make_term(smt::Equal, sv.first, sv.second));
    }
    auto state_expr = solver->make_term(smt::And, state_expr_vec);
    auto free_var = get_free_variables(state_expr);

    for(const auto& var:free_var){
        if(var->to_string() == "v1")
            auto v1 = var;
        else if(var->to_string() == "v2")
            auto v2 = var;
        else if(var->to_string() == "a")
            auto a = var;
        else if(var->to_string() == "b")
            auto b = var;
        else if(var->to_string() == "c")
            auto c = var;
    }

    // tag0 - tag0
    InvGroup inv_group0(0, tag0, branch_list, solver);
    inv_group0.branch2state();
    auto inv_dedup0 = inv_group0.inv_dedup();
    auto inv_l0 = solver->make_term(smt::Or, inv_dedup0);

    // tag0 - tag1
    InvGroup inv_group1(1, tag1, branch_list, solver);
    inv_group1.branch2state();
    auto inv_dedup1 = inv_group1.inv_dedup();
    auto inv_l1 = solver->make_term(smt::Or, inv_dedup1);

    // tag1 - tag1
    InvGroup inv_group2(2, tag1, branch_list, solver);
    inv_group2.branch2state();
    auto inv_dedup2 = inv_group2.inv_dedup();
    auto inv_l2 = solver->make_term(smt::Or, inv_dedup2);

    // tag1 - tag2
    InvGroup inv_group3(3, tag2, branch_list, solver);
    inv_group3.branch2state();
    auto inv_dedup3 = inv_group3.inv_dedup();
    auto inv_l3 = solver->make_term(smt::Or, inv_dedup3);

    // tag2 - tag2
    InvGroup inv_group4(4, tag2, branch_list, solver);
    inv_group4.branch2state();
    auto inv_group4_l4 = inv_group4.inv_group();
    auto inv_dedup4 = inv_group4.inv_dedup();
    auto inv_l4 = solver->make_term(smt::Or, inv_dedup4);
    auto inv_l4_prop = inv_group4.extract_prop("stage3");

    // tag2 - tag3
    InvGroup inv_group5(5, tag3, branch_list, solver);
    inv_group5.branch2state();
    auto inv_group5_l5 = inv_group5.inv_group();
    auto inv_dedup5 = inv_group5.inv_dedup();
    auto inv_l5 = solver->make_term(smt::Or, inv_dedup5);

    // tag3 - tag3
    InvGroup inv_group6(6, tag3, branch_list, solver);
    inv_group6.branch2state();
    auto inv_dedup6 = inv_group6.inv_dedup();
    auto inv_l6 = solver->make_term(smt::Or, inv_dedup6);


    // begin inv check
    auto trans_sts = sts.trans();
    smt::TermVec assume_vec;
    for(const auto& cons:sts.constraints()){
        assume_vec.push_back(solver->make_term(smt::Equal, cons.first, solver->make_term(cons.second)));
    }
    assume_vec.push_back(trans_sts);
    auto trans = solver->make_term(smt::And, assume_vec);

    smt::UnorderedTermMap var_to_next = {};
    for(const auto& sv:map){
        auto var = sv.second;
        auto st = var->get_sort();
        var_to_next[var] = solver->make_symbol(var->to_string()+"_next", st);
    }

    smt::TermVec next_expr_vec = {};
    for(const auto& var:var_to_next){
        next_expr_vec.push_back(solver->make_term(smt::Equal, var.second, var.first));
    }

    auto trans_nxt = solver->make_term(smt::And, next_expr_vec);
    auto trans_update = solver->make_term(smt::And, {trans, trans_nxt});

    auto asmpt_rst1 = solver->make_term(smt::Equal, rst, solver->make_term(0));
    auto asmpt_rst2 = solver->make_term(smt::Equal, solver->substitute(rst, sts.state_updates()), solver->make_term(0));

    auto one_hot_tag = one_hot({tag0, tag1, tag2, tag3}, solver);
    auto asmpt_tag = solver->make_term(smt::And, one_hot_tag);

    cout << asmpt_tag->to_string() << endl;

    auto asmpt = solver->make_term(smt::And, {asmpt_rst1, asmpt_rst2, asmpt_tag});

    smt::Term prop = {};
    if(btor_parser.propvec().size() == 1){
        prop = btor_parser.propvec().at(0);
    }
    else{
        prop = solver->make_term(smt::And, btor_parser.propvec());
    }

    cout << prop->to_string() << endl;








    auto end = system_clock::now();
    auto duration = duration_cast<seconds>(end-start);
    
    cout << "Program running time: " << double(duration.count())*seconds::period::num/seconds::period::den << " (s)" << endl;
}
