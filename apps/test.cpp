#include <chrono>
#include "assert.h"
#include "config/testpath.h"
#include "frontend/btor2_encoder.h"
#include "framework/symsim.h"
#include "framework/ts.h"
#include "smt-switch/boolector_factory.h"
#include "vexpparser/interpreter.h"
#include "astparser.h"
#include <iostream>
#include <sstream>

using namespace wasim;
using namespace smt;

verilog_expr::VExprAst::VExprAstPtr get_local_max_width(const verilog_expr::VExprAst::VExprAstPtr& node, wasim::SymbolicExecutor& sim, std::vector<int>& width_vec){
    
    verilog_expr::VExprAst::VExprAstPtr new_ast;
    verilog_expr::VExprAst::VExprAstPtrVec child_node_vec;

    if (!node) {
      throw std::invalid_argument("Null AST node");
    }

    switch (node->get_op()) {
      case verilog_expr::voperator::MK_VAR: {

            auto var_sort = sim.var(node -> to_verilog()) -> get_sort();
            int width = var_sort -> get_width();
            width_vec.push_back(width);

            new_ast = node;
            break;
      }

      default:{
            if(node -> get_child_cnt() == 1)
            {
              get_local_max_width(node->get_child().at(0), sim, width_vec);
              new_ast = node;
              break;
            }
            else if(node -> get_child_cnt() == 2)
            {
              get_local_max_width(node->get_child().at(0), sim, width_vec);
              get_local_max_width(node->get_child().at(1), sim, width_vec);
              new_ast = node;
              break;
            }
            else if(node -> get_child_cnt() == 3)
            {
              get_local_max_width(node->get_child().at(0), sim, width_vec);
              get_local_max_width(node->get_child().at(1), sim, width_vec);
              get_local_max_width(node->get_child().at(2), sim, width_vec);
              new_ast = node;
              break;
            }
      }
    }

    return new_ast;
}

verilog_expr::VExprAst::VExprAstPtr check_ast_soft(const verilog_expr::VExprAst::VExprAstPtr& node, wasim::SymbolicExecutor& sim, int& max_width)
{
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
              break;
            }
      }
      case verilog_expr::voperator::MK_VAR: {

            auto var_sort = sim.var(node -> to_verilog()) -> get_sort();
            auto var_sort_width = var_sort -> get_width();
            if(var_sort_width == max_width){
              new_ast = node;
              break;
            }
            else{
              auto concat_width = max_width - var_sort_width; 
              verilog_expr::VExprAst::VExprAstPtr sub_node = node -> MakeBinaryAst(verilog_expr::voperator::CONCAT, node, node -> MakeConstant(0, 0, std::to_string(concat_width)));
              new_ast = sub_node;
              break;
            }
      }
      default:{
            if(node -> get_child_cnt() == 0)
            {
              new_ast = node;
              break;
            }
            if(node -> get_child_cnt() == 1)
            {
              child_node_vec.push_back(check_ast_soft(node->get_child().at(0), sim, max_width));
              new_ast = node -> MakeCopyWithNewChild(child_node_vec);
              break;
            }
            else if(node -> get_child_cnt() == 2)
            {
              child_node_vec.push_back(check_ast_soft(node->get_child().at(0), sim, max_width));
              child_node_vec.push_back(check_ast_soft(node->get_child().at(1), sim, max_width));
              new_ast = node -> MakeCopyWithNewChild(child_node_vec);
              break;
            }
            else if(node -> get_child_cnt() == 3)
            {
              child_node_vec.push_back(check_ast_soft(node->get_child().at(0), sim, max_width));
              child_node_vec.push_back(check_ast_soft(node->get_child().at(1), sim, max_width));
              child_node_vec.push_back(check_ast_soft(node->get_child().at(2), sim, max_width));
              new_ast = node -> MakeCopyWithNewChild(child_node_vec);
              break;
            }
      }
    }

    return new_ast;
}

verilog_expr::VExprAst::VExprAstPtr check_ast(const verilog_expr::VExprAst::VExprAstPtr& node, wasim::SymbolicExecutor& sim)
{
  verilog_expr::VExprAst::VExprAstPtr new_ast;
    verilog_expr::VExprAst::VExprAstPtrVec child_node_vec;

    if (!node) {
      throw std::invalid_argument("Null AST node");
    }

    switch (node->get_op()) {

      case verilog_expr::voperator::L_EQ: {
            std::vector<int> width_vec;
            int local_max_width = 0;
            get_local_max_width(node, sim, width_vec);
            for(auto &width : width_vec)
            {
              if(local_max_width < width)
              {
                local_max_width = width;
              }
            }
            new_ast = check_ast_soft(node, sim, local_max_width);
            break;
      }

      default:{
            if(node -> get_child_cnt() == 0)
            {
              new_ast = node;
              break;
            }
            if(node -> get_child_cnt() == 1)
            {
              child_node_vec.push_back(check_ast(node->get_child().at(0), sim));
              new_ast = node -> MakeCopyWithNewChild(child_node_vec);
              break;
            }
            else if(node -> get_child_cnt() == 2)
            {
              child_node_vec.push_back(check_ast(node->get_child().at(0), sim));
              child_node_vec.push_back(check_ast(node->get_child().at(1), sim));
              new_ast = node -> MakeCopyWithNewChild(child_node_vec);
              break;
            }
            else if(node -> get_child_cnt() == 3)
            {
              child_node_vec.push_back(check_ast(node->get_child().at(0), sim));
              child_node_vec.push_back(check_ast(node->get_child().at(1), sim));
              child_node_vec.push_back(check_ast(node->get_child().at(2), sim));
              new_ast = node -> MakeCopyWithNewChild(child_node_vec);
              break;
            }
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
            result = solver->make_term(Equal, left, right);
            break;
        }
        // ite ? :
        case verilog_expr::voperator::TERNARY: {
            if (node->get_child_cnt() != 3) {
                throw std::runtime_error("node requires three childs");
            }
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term mid = ast2term(solver, node->get_child().at(1), sim);
            Term right = ast2term(solver, node->get_child().at(2), sim);
            result = solver->make_term(Ite, left, mid, right);
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
        // * 
        case verilog_expr::voperator::STAR: {
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term right = ast2term(solver, node->get_child().at(1), sim);
            result = solver->make_term(BVMul, left, right);
            break;
        }
        // bit and &
        case verilog_expr::voperator::B_AND: {
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term right = ast2term(solver, node->get_child().at(1), sim);
            result = solver->make_term(BVAnd, left, right);
            break;
        }
        // bit or |
        case verilog_expr::voperator::B_OR: {
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term right = ast2term(solver, node->get_child().at(1), sim);
            result = solver->make_term(BVOr, left, right);
            break;
        }
        // bit xor ^
        case verilog_expr::voperator::B_XOR: {
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term right = ast2term(solver, node->get_child().at(1), sim);
            result = solver->make_term(BVXor, left, right);
            break;
        }
        // bit not ~
        case verilog_expr::voperator::B_NEG: {
            Term left = ast2term(solver, node->get_child().at(0), sim);
            result = solver->make_term(BVNot, left);
            break;
        }
        // <<
        case verilog_expr::voperator::LSL: {
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term right = ast2term(solver, node->get_child().at(1), sim);

            auto left_width = (left -> get_sort()) -> get_width();
            auto lsl_width = log2(left_width);
            Sort lsl_sort = solver->make_sort(BV, lsl_width);
            Term lsl_term = solver->make_term(right -> to_int(), lsl_sort);
            result = solver->make_term(BVShl, left, lsl_term);  // [boolector] bit-width of 'e0(left)' must be a power of 2
            break;
        }
        // >>
        case verilog_expr::voperator::LSR: {
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term right = ast2term(solver, node->get_child().at(1), sim);

            auto left_width = (left -> get_sort()) -> get_width();
            auto lsr_width = log2(left_width);
            Sort lsr_sort = solver->make_sort(BV, lsr_width);
            Term lsr_term = solver->make_term(right -> to_int(), lsr_sort);
            result = solver->make_term(BVLshr, left, lsr_term); // [boolector] bit-width of 'e0(left)' must be a power of 2
            break;
        }
        // AT @
        case verilog_expr::voperator::AT: {
            Term left = ast2term(solver, node->get_child().at(0), sim);
            Term right = ast2term(solver, node->get_child().at(1), sim);

            //make sure not make same symbol
            auto string_name = node -> get_child().at(0) -> to_verilog() + "@" + std::to_string(right -> to_int());
            try{
               solver -> get_symbol(string_name);
            }
            catch(const IncorrectUsageException &e){
              result = solver->make_symbol(string_name, left -> get_sort());
            }

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
            //new for different width
            int n = std::stoi(node -> to_verilog());
            int width = 0;
            while (n > 0)
            {
                n >>= 1; 
                width++;
            }
            if(width == 0)
            {
              width = 1;
            }
            Sort const_int = solver->make_sort(BV, width); // there we use BV 32 substitude int, because boolector not support int type
            result = solver -> make_term(std::stoi(node -> to_verilog()), const_int);
            // std::cout << node -> to_verilog() << "|" << width << "|" << std::stoi(node -> to_verilog()) << std::endl;
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

  sim.init();

    // assert parse
  std::string my_assertion = "out@2 == a@0 + b@1";

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
  auto assertion_ast = intp.GetAstRoot();


    //get information from ast (get var, max cycle, term map)
  astparser::AstParser test(assertion_ast, sim);
  bool rst_en0 = 0;                             //if input signal have not rst, give 0;  if 1 -> rst = 0, else 0 -> rst = rst1(symbolic)
  test.sim_and_get_term(sim, sts, rst_en0);
  test.print_term_map();

  std::cout << "vexpparser ast : " << assertion_ast << std::endl;
    // make new ast tree (new version local width
  auto new_ass_ast = check_ast(assertion_ast, sim);
  std::cout << "sort new ast : " << new_ass_ast << std::endl;

    //ast transformer to term
  Term my_assertion_term = ast2term(solver, new_ass_ast, sim);
  std::cout << "my assertion term (original term) :" << my_assertion_term << std::endl;

      //create substitution map
  auto substitution_map = test.create_substitution_map(solver);

    //substitude term
  Term my_assertion_term_sub =  solver -> substitute(my_assertion_term, substitution_map);
  std::cout << "my assertion term (substitude symbolic term) :" << my_assertion_term_sub << std::endl;

    //check sat
  Term not_assertion_term = solver -> make_term(Not,my_assertion_term_sub);
  solver -> assert_formula(not_assertion_term);
  std::cout << "term check sat result : " << solver -> check_sat() << std::endl;

  if( (solver -> check_sat()).result == smt::SAT )
  {
    std::cout << "assert(" + my_assertion + ") : the equation isn't always correct and has counter-example" << std::endl;
  }
  else if((solver -> check_sat()).result == smt::UNSAT )
  {
    std::cout << "assert(" + my_assertion + ") : the equation is always correct" << std::endl;
  }

  return 0;

}


