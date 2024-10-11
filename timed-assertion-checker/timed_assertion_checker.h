#pragma once 
#include <string>
#include <unordered_map>

#include "vexpparser/interpreter.h"
#include "smt-switch/smt.h"
#include "framework/symsim.h"


namespace tac{

class TimedAssertionChecker {
public:
    struct Variable 
    {
        std::string fullname; // "a@2"
        std::string name;     // "a"
        int cycle;            // 2
        int width;            // get_width
    };
    std::unordered_map<std::string, smt::Term> ass_termmap;

    // parse verilog -> ast, parse ast -> get var, max cycle from wasim 
    TimedAssertionChecker(const std::string verilog_assertion, wasim::TransitionSystem & ts, wasim::SymbolicExecutor& sim, smt::SmtSolver solver) :ts_(ts), sim_(sim), solver_(solver)
        {parse_verilog(verilog_assertion); parse_ast(ast); parse_max_cycle();}

    //sim and catch var symbolic term
    void sim_max_step(bool& rst_en0);

    //after sim_max_step, print var <-> symbolic map 
    void print_term_map();

    //make assertion term
    void make_assertion_term();

    //ast->term and assert formula term
    void assert_formula();
    
    //check sat
    void check_sat();

    //get assertion term
    smt::Term get_assertion_term(){return assertion_term;}

    //get substitution assertion term
    smt::Term get_sub_assertion_term(){return assertion_term_sub;}

    //get ast
    verilog_expr::VExprAst::VExprAstPtr get_ast() const {return ast;}

    //get sim var struct
    std::vector<Variable> get_var_vec() const {return var_struct_vec;}

    //get max sim cycle
    int get_max_cycle() const {return max_cycle;}

protected:
    wasim::TransitionSystem & ts_;
    wasim::SymbolicExecutor& sim_;
    smt::SmtSolver solver_;

private:
    std::string verilog_assertion_;
    verilog_expr::VExprAst::VExprAstPtr ast;
    std::vector<Variable> var_struct_vec;
    int max_cycle;
    smt::Term assertion_term;
    smt::Term assertion_term_sub;

    //parse verilog assertion
    void parse_verilog(const std::string& verilog_assertion);

    //parse to get variable struct
    void parse_ast(const verilog_expr::VExprAst::VExprAstPtr& ast);

    //parse to get max sim cycle
    void parse_max_cycle();

    //get substitution map (term) -> (state symbolic term)
    smt::UnorderedTermMap create_substitution_map();
};

}   //namespace tac