#include "framework/ts.h"
#include "framework/btor2_encoder.h"
#include "symsim.h"
#include "deps/smt-switch/local/include/smt-switch/boolector_factory.h"
#include "assert.h"
#include "framework/term_manip.h"

using namespace wasim;
// using namespace smt;
using namespace std;

int main(){


    std::string input_file = "/data/wenjifang/vpipe-mc/btor-bmc/example/pipe-no-stall.btor2";
    
    smt::SmtSolver s = smt::BoolectorSolverFactory::create(false);
    wasim::TransitionSystem ts(s);
    BTOR2Encoder btor_parser(input_file, ts);

    SymbolicExecutor executor(ts, s);
    // auto value_sort = s->make_sort(smt::BV, 1);
    // auto value_new = s->make_term(1,value_sort);
    // cout << value_new << endl;
    // auto value_sort2 = s->make_sort(smt::BV, 1);
    // auto value_new2 = s->make_term(1,value_sort);
    // cout << value_new2 << endl;

    // cout << "TS init: " << ts.init() << endl;
    cout << "\nTS trans: " << ts.trans() << endl;
    // cout << "\nTS input vars: " << endl;
    cout << " " << endl;
    for(auto var: ts.inputvars()){
        cout << var << endl;
    }
    cout << "\nTS state vars: " << endl;
    for(auto var: ts.state_updates_){
        cout << var.first << "  -->-- " << var.second << endl;
    }
    
    cout << "\nTS constrains: " << endl;
    for (auto vect : ts.constraints_){
        cout << vect.first << "bool: "<< vect.second << endl;
        }
    
    cout << "\nTS prop: " << endl;
    for (auto vect : btor_parser.propvec()){
        cout << vect << endl;
        }
    
    // // btor_paser.preprocess(input_file);
    // // btor_paser.parse(input_file);
    
    std::map<wasim::type_conv, wasim::type_conv> init_map = {\
        {"wen_stage1","v1"},
        {"wen_stage2","v2"},
        {"wen_stage3","v3"},
        {"stage1","a"},
        {"stage2","b"},
        {"stage3","c"}
        };
    
    auto map = executor.convert(init_map);
    executor.init(map);


    executor.print_current_step();
    executor.print_current_step_assumptions();

    cout << ts.init_ << endl;
    // // tag1
    executor.set_input(
        executor.convert({{"rst",0}}), {}
    );

    executor.sim_one_step();

    executor.print_current_step();
    executor.print_current_step_assumptions();

    cout << executor.history_choice_.back().UsedInSim_ << endl;

    // // // tag2
    executor.set_input(
        executor.convert({{"rst",0}}), {}
    );

    executor.sim_one_step();

    executor.print_current_step();
    executor.print_current_step_assumptions();

    // // // tag3
    executor.set_input(
        executor.convert({{"rst",1}}), {}
    );

    executor.sim_one_step();

    executor.print_current_step();
    executor.print_current_step_assumptions();

    cout << "\n\n\n" << endl;
    auto cons = ts.constraints_[1].first;
    cout << cons << endl;

    for(auto v : cons){
        cout << "v: " <<v << endl;
        for (auto v0 : v){
            cout << "v0: " << v0 << endl;
            cout << v0->is_symbol() << endl;
            for(auto v00 : v0){
                cout << "v00: "<<v00 << endl;
            }
        }
    }

    cout << "\n\n" << endl;
    auto arg1 = arg(cons);
    cout << arg1.size() << endl;


    auto cons1 = ts.init_;
    cout << "expr: " << cons1 << endl;
    auto free_var = get_free_variable(cons1);
    cout << "free_var_num: " << free_var.size() << endl;
    for(auto v : free_var){
        cout << v << endl;
    }

    
    
    
    // // cout << cons->begin() << endl;
    // cout << cons->get_sort()->to_string() << endl;
    

    return 0;
    
}
