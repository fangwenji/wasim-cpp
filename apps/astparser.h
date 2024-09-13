#pragma once 
#include <string>
#include <unordered_map>

#include "vexpparser/interpreter.h"
#include "smt-switch/smt.h"
#include "framework/symsim.h"


namespace astparser{

// parse ast to get var, max cycle, term map
class AstParser {
public:
    struct Variable 
    {
        std::string fullname; // "a@2"
        std::string name;     // "a"
        int cycle;            // 2
        int width;            // get_width
    };
    std::unordered_map<std::string, smt::Term> ass_termmap;

    //parser
    AstParser(const verilog_expr::VExprAst::VExprAstPtr& ast, wasim::SymbolicExecutor& sim) {ast_get_info(ast, sim); parse_max_cycle();}

    //get sim var struct
    const std::vector<Variable>& get_var_vec() const {return var_struct_vec;}

    //get max sim cycle
    int get_max_cycle() const {return max_cycle;}

    //sim and get var symbolic term
    void sim_and_get_term(wasim::SymbolicExecutor& sim, wasim::TransitionSystem& sts,  bool& rst_en0);

    //print var <-> symbolic term
    void print_term_map();

    //get substitution map (term) -> (state symbolic term)
    smt::UnorderedTermMap create_substitution_map(smt::SmtSolver& solver);


private:
    std::vector<Variable> var_struct_vec;
    int max_cycle;

    //parse to get max sim cycle
    void parse_max_cycle();
    
    //parse to get variable
    verilog_expr::VExprAst::VExprAstPtr ast_get_info(const verilog_expr::VExprAst::VExprAstPtr& node, wasim::SymbolicExecutor& sim);
};

}   //namespace astparser