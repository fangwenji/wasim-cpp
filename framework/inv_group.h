#include "symtraverse.h"
using namespace std;

namespace wasim
{
    smt::Term tobool(smt::Term expr, smt::SmtSolver& solver);


    class InvGroup
    {
    public:
        InvGroup(int layer, smt::Term tag, std::vector<std::vector<StateAsmpt>> branch_list, smt::SmtSolver & s) :
            layer_(layer), tag_(tag),
            branch_list_(branch_list), solver_(s)  {  }

        void branch2state();
        smt::TermVec state_group();
        // smt::TermVec asmpt_group();
        smt::TermVec inv_group();
        smt::TermVec state_dedup();
        smt::TermVec inv_dedup();
        smt::TermVec extract_prop(std::string var_name);

    private:
        int layer_;
        smt::Term tag_;
        std::vector<std::vector<StateAsmpt>> branch_list_;
        smt::SmtSolver solver_;
        smt::TermVec state_group_;
        smt::TermVec asmpt_group_;
        smt::TermVec inv_group_;
        smt::TermVec state_dedup_;
        smt::TermVec asmpt_dedup_;
        smt::TermVec inv_dedup_;
    }; // end of class ChoiceItem
}
