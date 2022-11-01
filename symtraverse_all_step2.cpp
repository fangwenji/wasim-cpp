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

using namespace wasim;
// using namespace smt;
using namespace std;
using namespace chrono;

smt::TermVec tag2asmpt_c2(std::string flag, SymbolicExecutor & executor, smt::SmtSolver & solver){
    smt::TermVec ret;
    smt::Term ret_term0, ret_term1, ret_term2;
    smt::Term lhs0, lhs1, lhs2;
    smt::Term rhs_0 = solver->make_term(0, solver->make_sort(smt::BV, 1));
    smt::Term rhs_1 = solver->make_term(1, solver->make_sort(smt::BV, 1));
    if(flag == "start-ex"){
        lhs0 = executor.var("rst");
        ret_term0 = solver->make_term(smt::Equal, lhs0, rhs_0);
        ret.push_back(ret_term0);  
        lhs1 = executor.var("dummy_reset");
        ret_term1 = solver->make_term(smt::Equal, lhs1, rhs_0);
        ret.push_back(ret_term1);  
    }
    else if(flag == "ex-ex"){
        lhs0 = executor.var("rst");
        ret_term0 = solver->make_term(smt::Equal, lhs0, rhs_0);
        ret.push_back(ret_term0);  
        lhs1 = executor.var("dummy_reset");
        ret_term1 = solver->make_term(smt::Equal, lhs1, rhs_0);
        ret.push_back(ret_term1);
        lhs2 = executor.var("RTL_ex_go");
        ret_term2 = solver->make_term(smt::Equal, lhs2, rhs_0);
        ret.push_back(ret_term2);
    }
    else if(flag == "ex-wb"){
        lhs0 = executor.var("rst");
        ret_term0 = solver->make_term(smt::Equal, lhs0, rhs_0);
        ret.push_back(ret_term0);  
        lhs1 = executor.var("dummy_reset");
        ret_term1 = solver->make_term(smt::Equal, lhs1, rhs_0);
        ret.push_back(ret_term1);
        lhs2 = executor.var("RTL_ex_go");
        ret_term2 = solver->make_term(smt::Equal, lhs2, rhs_1);
        ret.push_back(ret_term2);
    }
    else if(flag == "wb-wb"){
        lhs0 = executor.var("rst");
        ret_term0 = solver->make_term(smt::Equal, lhs0, rhs_0);
        ret.push_back(ret_term0);  
        lhs1 = executor.var("dummy_reset");
        ret_term1 = solver->make_term(smt::Equal, lhs1, rhs_0);
        ret.push_back(ret_term1);
        lhs2 = executor.var("RTL_wb_go");
        ret_term2 = solver->make_term(smt::Equal, lhs2, rhs_0);
        ret.push_back(ret_term2);
    }
    else if(flag == "wb-finish"){
        lhs0 = executor.var("rst");
        ret_term0 = solver->make_term(smt::Equal, lhs0, rhs_0);
        ret.push_back(ret_term0);  
        lhs1 = executor.var("dummy_reset");
        ret_term1 = solver->make_term(smt::Equal, lhs1, rhs_0);
        ret.push_back(ret_term1);
        lhs2 = executor.var("RTL_wb_go");
        ret_term2 = solver->make_term(smt::Equal, lhs2, rhs_1);
        ret.push_back(ret_term2);
    }
    else{
        cout << "<ERROR> Wrong tag transition format!" << endl;
        assert(false);
    }
    return {solver->make_term(smt::And, ret)};
}

int main(){
    auto start = system_clock::now();
    std::unordered_set<std::string> base_sv = {"RTL_id_ex_operand1","RTL_id_ex_operand2","RTL_id_ex_op","RTL_id_ex_rd","RTL_id_ex_reg_wen","RTL_ex_wb_val","RTL_ex_wb_rd","RTL_ex_wb_reg_wen","RTL_id_ex_valid","RTL_ex_wb_valid",\
    "RTL_registers[0]","RTL_registers[1]","RTL_registers[2]","RTL_registers[3]",\
      "RTL_scoreboard[0]","RTL_scoreboard[1]","RTL_scoreboard[2]","RTL_scoreboard[3]",\
        "__VLG_I_inst", "__VLG_I_inst_valid"};
    TraverseBranchingNode node0({make_pair("rst", 1)}, {});
    TraverseBranchingNode node1({}, {make_pair("RTL_ex_go", 1)});
    TraverseBranchingNode node2({}, {make_pair("RTL_wb_go", 1)});

    auto order = {node0, node1, node2};

    // This reference the reference path
    std::string input_file =  PROJECT_SOURCE_DIR "/design/testcase2-three_stage_pipe/problem_add.btor2";

    
    smt::SmtSolver solver = smt::BoolectorSolverFactory::create(false);
    solver->set_logic("QF_UFBV");
    solver->set_opt("incremental", "true");
    solver->set_opt("produce-models", "true");
    solver->set_opt("produce-unsat-assumptions", "true");
    TransitionSystem sts(solver);
    BTOR2Encoder btor_parser(input_file, sts);
    auto invar = sts.inputvars();
    for(const auto& var:invar){
        std::cout << var->to_string() << std::endl;
    }
    cout << "\n\n state var: \n" << endl;
    auto svar = sts.statevars();
    for(const auto& var:svar){
        std::cout << var->to_string() << std::endl;
    } 
    exit(1);

    SymbolicExecutor executor(sts, solver);

    assignment_type init_map = {
        {"RTL_id_ex_operand1","oper1"},
        {"RTL_id_ex_operand2","oper2"},
        {"RTL_id_ex_op","op"},
        {"RTL_id_ex_rd","rd1"},
        {"RTL_id_ex_reg_wen","w1"},
        {"RTL_ex_wb_val","ex_val"},
        {"RTL_ex_wb_rd","rd2"},
        {"RTL_ex_wb_reg_wen","w2"},
        {"RTL_id_ex_valid","v1"},
        {"RTL_ex_wb_valid","v2"},
        {"RTL_registers[0]","reg0"},
        {"RTL_registers[1]","reg1"},
        {"RTL_registers[2]","reg2"},
        {"RTL_registers[3]","reg3"},
        {"RTL_scoreboard[0]","s0"},
        {"RTL_scoreboard[1]","s1"},
        {"RTL_scoreboard[2]","s2"},
        {"RTL_scoreboard[3]","s3"},
        {"__VLG_I_inst", "inst"},
        {"__VLG_I_inst_valid","inst_v"},
        {"__ILA_I_inst","ila_inst"},
        {"__auxvar0__recorder_sn_condmet",0},
        {"__auxvar1__recorder_sn_condmet",0},
        {"__auxvar2__recorder_sn_condmet",0},
        {"__auxvar3__recorder_sn_condmet",0}
        };
    
    auto map = executor.convert(init_map);
    executor.init(map);
    std::vector<StateAsmpt> state_list;
    std::vector<std::vector<StateAsmpt>> branch_list;

    // step: start
    cout << "step: start" << endl;
    auto s_init = executor.get_curr_state();
    auto is_not_start0 = solver->make_term(smt::Equal, executor.var("__START__"), solver->make_term(0, solver->make_sort(smt::BV, 1)));
    auto is_not_start = executor.interpret_state_expr_on_curr_frame(is_not_start0);
    auto init_asmpt = s_init.asmpt_;
    init_asmpt.push_back(is_not_start);
    auto r_t = solver->check_sat_assuming(init_asmpt);
    smt::TermVec pop_vec = {};
    for(const auto& sv : s_init.sv_){
        auto s = sv.first;
        auto v = sv.second;
        if(base_sv.find(s->to_string()) == base_sv.end()){
            pop_vec.push_back(s);
        }
    }
    for(const auto& s:pop_vec){
        s_init.sv_.erase(s);
    }
    state_list.push_back(s_init);
    branch_list.push_back(state_list);
    s_init.print();
    s_init.print_assumptions();
    
    // step: start-ex
    cout << "step: start --> ex" << endl;
    auto asmpt_start_ex = tag2asmpt_c2("start-ex", executor, solver);
    extend_branch_next_phase(branch_list, executor, sts, base_sv, "start-ex", asmpt_start_ex, {{"__START__", 1}, {"ppl_stage_ex", 0}, {"ppl_stage_wb", 0}, {"ppl_stage_finish", 0}}
, order, solver);
/*
    // step: tag1 --> tag1
    cout << "\n\n\nstep: tag1 --> tag1" << endl;
    extend_branch_same_phase(branch_list, executor, sts, base_sv, "tag1_1", {{"tag0",0}, {"tag1",1}, {"tag2",0}, {"tag3",0}}, order, solver);

    // step: tag1 --> tag2
    cout << "\n\n\nstep: tag1 --> tag2" << endl;
    extend_branch_next_phase(branch_list, executor, sts, base_sv, "tag1_2", {{"tag0",0}, {"tag1",1}, {"tag2",0}, {"tag3",0}}, order, solver);

    // step: tag2 --> tag2
    cout << "\n\n\nstep: tag2 --> tag2" << endl;
    extend_branch_same_phase(branch_list, executor, sts, base_sv, "tag2_2", {{"tag0",0}, {"tag1",0}, {"tag2",1}, {"tag3",0}}, order, solver);

    // step: tag2 --> tag3
    cout << "\n\n\nstep: tag2 --> tag3" << endl;
    extend_branch_next_phase(branch_list, executor, sts, base_sv, "tag2_3", {{"tag0",0}, {"tag1",0}, {"tag2",1}, {"tag3",0}}, order, solver);

    // step: tag3 --> tag3
    cout << "\n\n\nstep: tag3 --> tag3" << endl;
    extend_branch_same_phase(branch_list, executor, sts, base_sv, "tag3_3", {{"tag0",0}, {"tag1",0}, {"tag2",0}, {"tag3",1}}, order, solver);


    auto end = system_clock::now();
    auto duration = duration_cast<seconds>(end-start);

    cout << "Program running time: " << double(duration.count())*seconds::period::num/seconds::period::den << " (s)" << endl;

    
    // std::string smt_file = PROJECT_SOURCE_DIR "/smtlib_result3.smt";
    // PropertyInterface pi(smt_file, solver);
    // auto expr = pi.return_defs();
    */
}
