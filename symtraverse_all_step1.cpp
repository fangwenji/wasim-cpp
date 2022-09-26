#include "framework/ts.h"
#include "framework/btor2_encoder.h"
#include "symsim.h"
#include "deps/smt-switch/local/include/smt-switch/boolector_factory.h"
#include "assert.h"
#include "framework/term_manip.h"
#include "framework/symtraverse.h"
#include "term_manip.h"
#include "traverse_manip.h"

using namespace wasim;
// using namespace smt;
using namespace std;

int main(){
    std::vector<std::string> base_sv = {"wen_stage1", "wen_stage2", "stage1", "stage2", "stage3"};
    TraverseBranchingNode node0({make_pair("rst", 1)}, {});
    TraverseBranchingNode node1({}, {make_pair("stage1_go", 1)});
    TraverseBranchingNode node2({}, {make_pair("stage2_go", 1)});
    TraverseBranchingNode node3({}, {make_pair("stage3_go", 1)});

    auto order = {node0, node1, node2, node3};

    std::string input_file = "/data/wenjifang/WASIM/design/testcase1-simple_pipe/simple_pipe.btor2";

    
    smt::SmtSolver solver = smt::BoolectorSolverFactory::create(false);
    solver->set_logic("QF_UFBV");
    solver->set_opt("incremental", "true");
    solver->set_opt("produce-models", "true");
    solver->set_opt("produce-unsat-assumptions",
    "true");
    wasim::TransitionSystem sts(solver);
    BTOR2Encoder btor_parser(input_file, sts);

    SymbolicExecutor executor(sts, solver);

    std::map<wasim::type_conv, wasim::type_conv> init_map = {\
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

    // step: tag0 --> tag0
    cout << "step: tag0 --> tag0" << endl;

    extend_branch_init(branch_list, executor, sts, base_sv, "tag0_0", order, solver);

    



    


    



}