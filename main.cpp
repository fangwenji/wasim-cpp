#include "framework/ts.h"
#include "framework/btor2_encoder.h"
#include "deps/smt-switch/local/include/smt-switch/boolector_factory.h"
#include "assert.h"

using namespace pono;
using namespace smt;
using namespace std;

int main(int argc, char** argv){

    if(argv[1] == NULL){
        cout << "NO Input File!" << endl; 
    }
    else{
        cout << "Input File: " << argv[1] << endl;
    }

    std::string input_file;
    input_file = argv[1];
    
    

    SmtSolver s = BoolectorSolverFactory::create(false);
    pono::TransitionSystem ts(s);
    BTOR2Encoder btor_paser(input_file, ts);
    btor_paser.preprocess(input_file);
    btor_paser.parse(input_file);
    



    // cout << "input0: " << argv[0] << endl;
    // cout << "input1: " << argv[1] << endl;


    return 0;
}
