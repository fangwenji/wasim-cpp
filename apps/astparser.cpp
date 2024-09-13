#include "astparser.h"
namespace astparser {
// AstParser : parse ast to get var, max cycle, term map
verilog_expr::VExprAst::VExprAstPtr AstParser::ast_get_info(const verilog_expr::VExprAst::VExprAstPtr& node, wasim::SymbolicExecutor& sim){
    
    verilog_expr::VExprAst::VExprAstPtr new_ast;
    verilog_expr::VExprAst::VExprAstPtrVec child_node_vec;
    Variable var_struct;

    if (!node) {
      throw std::invalid_argument("Null AST node");
    }

    switch (node->get_op()) {

      case verilog_expr::voperator::AT: {

            std::string fullname = {node -> get_child().at(0) -> to_verilog() + "@" + node -> get_child().at(1) -> to_verilog()};
            std::string name = node -> get_child().at(0) -> to_verilog();
            int cycle = std::stoi(node -> get_child().at(1) -> to_verilog());
            auto var_sort = sim.var(node -> get_child().at(0) -> to_verilog()) -> get_sort();
            int width = var_sort -> get_width();

            var_struct = {fullname, name, cycle, width};
            var_struct_vec.push_back(var_struct);
            new_ast = node;
            break;
      }

      default:{
            if(node -> get_child_cnt() == 1)
            {
              ast_get_info(node->get_child().at(0), sim);
              new_ast = node;
              break;
            }
            else if(node -> get_child_cnt() == 2)
            {
              ast_get_info(node->get_child().at(0), sim);
              ast_get_info(node->get_child().at(1), sim);
              new_ast = node;
              break;
            }
            else if(node -> get_child_cnt() == 3)
            {
              ast_get_info(node->get_child().at(0), sim);
              ast_get_info(node->get_child().at(1), sim);
              ast_get_info(node->get_child().at(2), sim);
              new_ast = node;
              break;
            }
      }
    }

    return new_ast;
}

void AstParser::parse_max_cycle()
{
    max_cycle = 0;
    for(auto &var_struct : var_struct_vec)
    {
      if(max_cycle < var_struct.cycle)
      {
        max_cycle = var_struct.cycle;
      }
    }
}

void AstParser::sim_and_get_term(wasim::SymbolicExecutor& sim, wasim::TransitionSystem& sts, bool& rst_en0)
{
    int sim_cycle;
    const auto& vars = get_var_vec();
    
    //get input var for make vdict
    smt::UnorderedTermSet input_term_set = sts.inputvars();
    smt::Term clk_term = sim.var("clk");
    input_term_set.erase(clk_term);
    
    for (sim_cycle = 0; sim_cycle <= max_cycle; sim_cycle ++)
    {
        std::cout << "--------------------------------------@" << sim_cycle << std::endl;
        if(sim_cycle != 0)
        {
          sim.sim_one_step();
        }
        
        
        //make vdict for each input sim.convert
        wasim::assignment_type input_vdict;
        for(auto &input_var : input_term_set){
          input_vdict[input_var -> to_string()] = input_var -> to_string() + std::to_string(sim_cycle);
        }

        if(rst_en0){
          input_vdict["rst"] = 0;
        }


        auto inputmap = sim.convert(input_vdict); //{{"a", "a" + std::to_string(sim_cycle)}, {"b", "b" + std::to_string(sim_cycle)}}
        sim.set_input(inputmap, {}); 

        auto s = sim.get_curr_state();
        std::cout << s.print();  //print state value
        std::cout << s.print_assumptions(); //print state assumption

        int j;
        bool skip = false;
        for(j = 0; j < vars.size(); j++)
        { 
          if(vars[j].cycle == sim_cycle)
          { 
            //get term form input var
            for(const auto &input : inputmap) //  get the unod_map<term, term>, <term_name, term_symbolic_expression>
            {
              if( (input.first -> to_string()) == vars[j].name)
              {
                // std::cout << "we got the symbolic term " << vars[j].fullname << ": " << input.second << std::endl;
                ass_termmap[vars[j].fullname] = input.second;
                skip = true;
                break;
              }
            }

            if(skip){
              skip = false;
              continue;
            }

            //get term from sim var
            auto out_term = sim.var(vars[j].name);  //get var term name
            ass_termmap[vars[j].fullname] = sim.interpret_state_expr_on_curr_frame(out_term);
          }
        }
    }
}

void AstParser::print_term_map(){
  std::cout << "-----------------we got the symbolic term----------------" << std::endl;
  std::cout << "---------------------------------------------------------" << std::endl;
  std::cout << "term <-> symbolic term" << std::endl;
  for(const auto &var_term : ass_termmap)  //ass_termmap
  {
    std::cout << var_term.first << " <-> " << var_term.second << std::endl;
  }
  std::cout << "---------------------------------------------------------" << std::endl;
}

smt::UnorderedTermMap AstParser::create_substitution_map(smt::SmtSolver& solver){
  smt::UnorderedTermMap substitution_map;
  for(auto &var_struct : var_struct_vec){
    smt::Term origin_term = solver -> get_symbol(var_struct.fullname);
    smt::Term sub_term = ass_termmap[var_struct.fullname];
    substitution_map[origin_term] = sub_term;
  }
  return substitution_map;
}

}   //namespace astparser