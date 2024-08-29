#include <chrono>
#include "assert.h"
#include "config/testpath.h"
#include "frontend/btor2_encoder.h"
#include "framework/symsim.h"
#include "framework/ts.h"
#include "smt-switch/boolector_factory.h"
#include "vexpparser/interpreter.h"

#include <iostream>
#include <sstream>

using namespace wasim;
using namespace smt;

//print ast tree architecture
void traverse_and_print_ast(const verilog_expr::VExprAst::VExprAstPtr& node, int level = 0) {
    if (!node) {
        return;
    }
    std::string indent(level * 2, ' ');
    // std::cout << indent << "Node op: " << node -> get_op()<< std::endl;
    std::cout << indent << "Node : " << node << std::endl;
    // std::cout << indent << "Node Verilog: " << node->to_verilog() << std::endl;
    // traverse child ast
    for (const auto& child : node -> get_child() ){
        traverse_and_print_ast(child, level + 1);
    }
}

verilog_expr::VExprAst::VExprAstPtr check_ast_soft(const verilog_expr::VExprAst::VExprAstPtr& node, wasim::SymbolicExecutor& sim, int& max_width){
    
    verilog_expr::VExprAst::VExprAstPtr new_ast;
    verilog_expr::VExprAst::VExprAstPtrVec child_node_vec;

    if (!node) {
      throw std::invalid_argument("Null AST node");
    }

    switch (node->get_op()) {

      case verilog_expr::voperator::AT: {

            auto var_sort = sim.var(node -> get_child().at(0) -> to_verilog()) -> get_sort();
            auto var_sort_width = var_sort -> get_width();
            if(var_sort_width == max_width){
              new_ast = node;
              break;
            }
            else{
              auto concat_width = max_width - var_sort_width; 
              verilog_expr::VExprAst::VExprAstPtr sub_node = node -> MakeBinaryAst(verilog_expr::voperator::CONCAT, node, node -> MakeConstant(0, 0, std::to_string(concat_width)));
              new_ast = sub_node;
              // std::cout << new_ast << std::endl;
              break;
            }
      }
      default:{
            child_node_vec.push_back(check_ast_soft(node->get_child().at(0), sim, max_width));
            child_node_vec.push_back(check_ast_soft(node->get_child().at(1), sim, max_width));
            new_ast = node -> MakeCopyWithNewChild(child_node_vec);
            // std::cout << new_ast << std::endl;
            break;
      }
    }

    return new_ast;
}

smt::Term ast2term(SmtSolver& solver, const verilog_expr::VExprAst::VExprAstPtr& node, wasim::SymbolicExecutor& sim) {
    if (!node) {
        throw std::invalid_argument("Null AST node");
    }

    Term result;

    // check op
    switch (node->get_op()) {
        // ==
        case verilog_expr::voperator::L_EQ: {
            if (node->get_child_cnt() != 2) {
                throw std::runtime_error("node requires two childs");
            }
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term right = ast2term(solver, node->get_child().at(1), sim);
            // std::cout << left << std::endl;
            // std::cout << right << std::endl;
            result = solver->make_term(Equal, left, right);
            break;
        }
        // +
        case verilog_expr::voperator::PLUS: {
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term right = ast2term(solver, node->get_child().at(1), sim);
            result = solver->make_term(BVAdd, left, right);
            break;
        }
        // - 
        case verilog_expr::voperator::MINUS: {
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term right = ast2term(solver, node->get_child().at(1), sim);
            result = solver->make_term(BVSub, left, right);
            break;
        }
        // bit and
        case verilog_expr::voperator::B_AND: {
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term right = ast2term(solver, node->get_child().at(1), sim);
            result = solver->make_term(BVAnd, left, right);
            break;
        }
        // bit or
        case verilog_expr::voperator::B_OR: {
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term right = ast2term(solver, node->get_child().at(1), sim);
            result = solver->make_term(BVOr, left, right);
            break;
        }
        // bit xor
        case verilog_expr::voperator::B_XOR: {
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term right = ast2term(solver, node->get_child().at(1), sim);
            result = solver->make_term(BVXor, left, right);
            break;
        }
        // logic and
        case verilog_expr::voperator::L_AND: {
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term right = ast2term(solver, node->get_child().at(1), sim);
            result = solver->make_term(And, left, right);
            break;
        }
        // logic or
        case verilog_expr::voperator::L_OR: {
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term right = ast2term(solver, node->get_child().at(1), sim);
            result = solver->make_term(Or, left, right);
            break;
        }
        // AT @
        case verilog_expr::voperator::AT: {
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term right = ast2term(solver, node->get_child().at(1), sim);
            // std::cout << "left :" << left << std::endl;
            // std::cout << "right :" << right << std::endl;

            //make sure not make same symbol
            auto string_name = left -> to_string() + "@" + std::to_string(right -> to_int());
            try{
               solver -> get_symbol(string_name);
            }
            catch(const IncorrectUsageException &e){
              result = solver->make_symbol(string_name, left -> get_sort());
            }
            // result = solver->make_symbol(left -> to_string() + "@" + std::to_string(right -> to_int()), left -> get_sort());
            result = solver -> get_symbol(string_name);

            break;
        }
        // concat
        case verilog_expr::voperator::CONCAT: {
          
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term right = ast2term(solver, node->get_child().at(1), sim);
        
            Sort concat_sort = solver -> make_sort(BV, right -> to_int());
            Term concat_term = solver -> make_term(0, concat_sort);
            result = solver -> make_term(Concat, concat_term, left);
            break;
        }

        // mk_var
        case verilog_expr::voperator::MK_VAR: {
            result = sim.var(node -> to_verilog()); //get var term from sim
            break;
        }
        // mk_const
        case verilog_expr::voperator::MK_CONST: {
            // std::cout << node -> to_verilog() << std::endl;
            Sort const_int = solver->make_sort(BV, 32); // there we use bv 32 substitude int, because boolector not support int type
            result = solver -> make_term(std::stoi(node -> to_verilog()), const_int);
            break;
        }
        
        
        default:
            throw std::runtime_error("Unknown operation in AST");
    }

    return result;
}

int main() {


  SmtSolver solver = BoolectorSolverFactory::create(false);

  solver->set_logic("QF_UFBV");
  solver->set_opt("incremental", "true");
  solver->set_opt("produce-models", "true");
  solver->set_opt("produce-unsat-assumptions", "true");

  TransitionSystem sts(solver);
  BTOR2Encoder btor_parser("/home/cwb/work_github/wasim-cpp/design/test/adder.btor2", sts); //design/test/adder.btor2

  std::cout << sts.trans()->to_string() << std::endl; //print smt 

  SymbolicExecutor sim(sts, solver);
  
  /*------------------------------simulation--------------------------------*/
  // auto varmap = sim.convert( {{"rego", 0}} );
  sim.init();

    // assert parse
  std::string my_assertion = "out@2 == a@0 + b@1";

  AssTermParser ass_parser(my_assertion, sim);        //parse assertion to get variables and max cycle

  bool rst_en0 = 0;                                   //if input signal have not rst, give 0;  if 1 -> rst = 0, else 0 -> rst = rst1(symbolic)
  ass_parser.sim_and_get_term(sim, sts, rst_en0);     //sim max cycle and get variables symbolic term
  ass_parser.print_getterm();                         //print we got the symbolic term
  int max_width = ass_parser.get_max_width();


    //vexpparser (get the ast)
  Vexp::Interpreter intp;
  std::stringstream ss;
  ss << my_assertion;
  intp.switchInputStream(&ss);
  try{
    intp.parse();
  } catch (verilog_expr::VexpException &e) {
    std::cout << "AST constructor error:" << e.msg_ << std::endl;
  }
  auto ass_ast = intp.GetAstRoot();
  std::cout << "vexpparser ast : " << ass_ast << std::endl;
  // traverse_and_print_ast(ass_ast);  //can show ast architecture


    //make new ast tree
  auto new_ass_ast = check_ast_soft(ass_ast, sim, max_width);
  std::cout << "sort new ast : " << new_ass_ast << std::endl;

    //ast transformer to term
  Term my_assertion_term = ast2term(solver, new_ass_ast, sim);
  std::cout << "my assertion term (original term) :" << my_assertion_term << std::endl;
  
    //create substitution map
  auto substitution_map = ass_parser.create_substitution_map(solver);

    //substitude term
  Term my_assertion_term_sub =  solver -> substitute(my_assertion_term, substitution_map);
  std::cout << "my assertion term (substitude symbolic term) :" << my_assertion_term_sub << std::endl;


    //check sat
  Term not_assertion_term = solver -> make_term(Not,my_assertion_term_sub);
  solver -> assert_formula(not_assertion_term);
  std::cout << "term check sat result : " << solver -> check_sat() << std::endl;

  if( (solver -> check_sat()).result == smt::SAT )
  {
    std::cout << "assert(" + my_assertion + ") : the equation isn't always correct and has incorrect situation" << std::endl;
  }
  else if((solver -> check_sat()).result == smt::UNSAT )
  {
    std::cout << "assert(" + my_assertion + ") : the equation is always correct" << std::endl;
  }

  return 0;

}


