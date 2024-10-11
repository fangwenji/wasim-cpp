#include "timed_assertion_checker.h"

namespace tac {

int get_max_width(const verilog_expr::VExprAst::VExprAstPtr& ast, wasim::SymbolicExecutor& sim){
    
    if (!ast) {
      throw std::invalid_argument("Null AST node");
    }

    std::vector<int> width_vec;
    int max_width = 0;

    std::stack<verilog_expr::VExprAst::VExprAstPtr> node_stack;
    node_stack.push(ast);

    while (!node_stack.empty()) {
        auto node = node_stack.top();
        node_stack.pop();

        switch (node->get_op()) {
            case verilog_expr::voperator::AT: {
                auto var_sort = sim.var(node -> get_child().at(0) -> to_verilog()) -> get_sort();
                int width = var_sort -> get_width();
                width_vec.push_back(width);
                break;
            }
            case verilog_expr::voperator::MK_VAR: {
                auto var_sort = sim.var(node -> to_verilog()) -> get_sort();
                int width = var_sort -> get_width();
                width_vec.push_back(width);
                break;
            }
            case verilog_expr::voperator::MK_CONST: {
                auto current_const = verilog_expr::VExprAstConstant::cast_ptr(node);
                auto const_value_tuple = current_const -> get_constant();
                auto width = std::get<1>(const_value_tuple);
                width_vec.push_back(width);
                break;
            }
            case verilog_expr::voperator::INDEX: {
                width_vec.push_back(1);
                break;
            }
            case verilog_expr::voperator::RANGE_INDEX: {
                auto msb = std::stoi(node -> get_child().at(1) -> to_verilog());
                auto lsb = std::stoi(node -> get_child().at(2) -> to_verilog());
                width_vec.push_back(msb - lsb + 1);
                break;
            }
            default: {
                for (size_t i = 0; i < node -> get_child_cnt(); ++i) {
                    node_stack.push(node -> get_child().at(i));
                }
                break;
            }
        }
        
    }

  for(auto &width : width_vec){
    if(max_width < width)
      { 
        max_width = width;
      }
  }

  return max_width;
}

verilog_expr::VExprAst::VExprAstPtr set_extend_width(verilog_expr::VExprAst::VExprAstPtr& ast, wasim::SymbolicExecutor& sim, int& max_width)
{
    if (!ast) {
        throw std::invalid_argument("Null AST node");
    }

    std::stack<verilog_expr::VExprAst::VExprAstPtr> node_stack;
    node_stack.push(ast);

    while (!node_stack.empty()) {
        auto current_node = node_stack.top();
        node_stack.pop();
            switch (current_node->get_op()) {
                case verilog_expr::voperator::AT: {
                  auto var_sort = sim.var(current_node -> get_child().at(0) -> to_verilog()) -> get_sort();
                  auto var_sort_width = var_sort -> get_width();
                  if(var_sort_width == max_width){
                    break;
                  }
                  else{
                    auto concat_width = max_width - var_sort_width; 
                    current_node -> set_concat_width(concat_width);
                    break;
                  }
                }
                case verilog_expr::voperator::MK_VAR: {
                  auto var_sort = sim.var(current_node -> to_verilog()) -> get_sort();
                  auto var_sort_width = var_sort -> get_width();
                  if(var_sort_width == max_width){
                    break;
                  }
                  else{
                    auto concat_width = max_width - var_sort_width; 
                    current_node -> set_concat_width(concat_width);
                    break;
                  }
                }
                case verilog_expr::voperator::MK_CONST: {
                  auto current_const = verilog_expr::VExprAstConstant::cast_ptr(current_node);
                  auto const_value_tuple = current_const -> get_constant();
                  auto width = std::get<1>(const_value_tuple);
                  if(width == max_width){
                    break;
                  }
                  else{
                    auto concat_width = max_width - width;
                    current_node -> set_concat_width(concat_width);
                    break;
                  }
                }
                case verilog_expr::voperator::INDEX: {
                  if(max_width == 1){
                    break;
                  }
                  else{
                    auto concat_width = max_width - 1;
                    current_node -> set_concat_width(concat_width);
                    break;
                  }
                }
                case verilog_expr::voperator::RANGE_INDEX: {
                  auto msb = std::stoi(current_node -> get_child().at(1) -> to_verilog());
                  auto lsb = std::stoi(current_node -> get_child().at(2) -> to_verilog());
                  
                  if(max_width == (msb - lsb + 1)){
                    break;
                  }
                  else{
                    auto concat_width = max_width - (msb - lsb + 1);
                    current_node -> set_concat_width(concat_width);
                    break;
                  }
                }
                default: {
                  for (size_t i = 0; i < current_node -> get_child_cnt(); ++i) {
                    node_stack.push(current_node -> get_child().at(i));
                  }
                  break;
                }
            }
            
    }
    std::cout << "local AST set width extension : done" << std::endl;
    return ast;
}

verilog_expr::VExprAst::VExprAstPtr traverse_ast_and_set_extend_width(verilog_expr::VExprAst::VExprAstPtr& node, wasim::SymbolicExecutor& sim)
{
    if (!node) {
        throw std::invalid_argument("Null AST node");
    }

    std::stack<std::pair<verilog_expr::VExprAst::VExprAstPtr, int>> node_stack;
    
    node_stack.push({node, 0}); 

    while (!node_stack.empty()) {
        auto [current_node, child_index] = node_stack.top();

        if (child_index < current_node->get_child_cnt()) {
            node_stack.top().second++;
            node_stack.push({current_node->get_child().at(child_index), 0});
        } 
        else {
            node_stack.pop(); 

            switch (current_node->get_op()) {
                case verilog_expr::voperator::L_EQ: {
                  verilog_expr::VExprAst::VExprAstPtrVec new_child;
                  
                  int local_max_width = get_max_width(current_node, sim);
                  set_extend_width(current_node, sim, local_max_width);
                  
                  break;
                }
                default: {
                  // std::cout << "current_node: " << current_node << std::endl;
                  break;
                }
            }
            
        }
    }
    
    return node;
}

smt::Term ast2term(smt::SmtSolver& solver, const verilog_expr::VExprAst::VExprAstPtr& node, wasim::SymbolicExecutor& sim) {
    if (!node) {
      throw std::invalid_argument("Null AST node");
    }

    std::stack<std::pair<const verilog_expr::VExprAst::VExprAstPtr, int>> stack; 
    stack.push({node, 0});
    std::unordered_map<verilog_expr::VExprAst::VExprAstPtr, smt::Term> terms;

    while (!stack.empty()) {
        auto [current, child_index] = stack.top();
        
        if (child_index < current->get_child_cnt()) {
            stack.top().second++;
            stack.push({current->get_child().at(child_index), 0});
        } 
        else {
            stack.pop(); 

            switch (current->get_op()) {
                // == 
                case verilog_expr::voperator::L_EQ: {
                    if (current->get_child_cnt() != 2) {
                        throw std::runtime_error("Equal node requires two childs");
                    }
                    terms[current] = solver->make_term(smt::Equal, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    break;
                }
                // ? :
                case verilog_expr::voperator::TERNARY: {
                    if (current->get_child_cnt() != 3) {
                        throw std::runtime_error("TERNARY node requires three childs");
                    }
                    terms[current] = solver->make_term(smt::Ite, terms[current->get_child().at(0)], terms[current->get_child().at(1)], terms[current->get_child().at(2)]);
                    break;
                }
                // +
                case verilog_expr::voperator::PLUS: {
                    terms[current] = solver->make_term(smt::BVAdd, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    break;
                }
                // -
                case verilog_expr::voperator::MINUS: {
                    terms[current] = solver->make_term(smt::BVSub, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    break;
                }
                // *
                case verilog_expr::voperator::STAR: {
                    terms[current] = solver->make_term(smt::BVMul, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    break;
                }
                // &
                case verilog_expr::voperator::B_AND: {
                    terms[current] = solver->make_term(smt::BVAnd, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    break;
                }
                // |
                case verilog_expr::voperator::B_OR: {
                    terms[current] = solver->make_term(smt::BVOr, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    break;
                }
                // ^
                case verilog_expr::voperator::B_XOR: {
                    terms[current] = solver->make_term(smt::BVXor, terms[current->get_child().at(0)], terms[current->get_child().at(1)]);
                    break;
                }
                // ~
                case verilog_expr::voperator::B_NEG: {
                    terms[current] = solver->make_term(smt::BVNot, terms[current->get_child().at(0)]);
                    break;
                }
                // @
                case verilog_expr::voperator::AT: {
                    if( (current -> get_concat_width()) != 0){
                        auto string_name = current->get_child().at(0)->to_verilog() + "@" + current->get_child().at(1)->to_verilog();
                        try{
                          solver -> get_symbol(string_name);
                        }
                        catch(const IncorrectUsageException &e){
                          terms[current] = solver->make_symbol(string_name, terms[current->get_child().at(0)] -> get_sort());
                        }
                        terms[current] = solver -> get_symbol(string_name);

                        smt::Sort concat_sort = solver -> make_sort(smt::BV, current -> get_concat_width());
                        smt::Term concat_term = solver -> make_term(0, concat_sort);
                        terms[current] = solver -> make_term(smt::Concat, concat_term, solver -> get_symbol(string_name));
                        break;
                    }
                    
                    auto string_name = current->get_child().at(0)->to_verilog() + "@" + current->get_child().at(1)->to_verilog();
                    try{
                       solver -> get_symbol(string_name);
                    }
                    catch(const IncorrectUsageException &e){
                      terms[current] = solver->make_symbol(string_name, terms[current->get_child().at(0)] -> get_sort());
                    }
                    terms[current] = solver -> get_symbol(string_name);
                    break;
                }
                // concat, extend width
                case verilog_expr::voperator::CONCAT: {
                    smt::Term left = terms[current->get_child().at(0)];
                    smt::Term right = terms[current->get_child().at(1)];

                    smt::Sort concat_sort = solver -> make_sort(smt::BV, right -> to_int());
                    smt::Term concat_term = solver -> make_term(0, concat_sort);
                    terms[current] = solver -> make_term(smt::Concat, concat_term, left);
                    break;
                }
                // index []
                case verilog_expr::voperator::INDEX: {
                    if( (current -> get_concat_width()) != 0){
                        smt::Term left = terms[current->get_child().at(0)];
                        smt::Term right = terms[current->get_child().at(1)];
                        smt::Op extract_op = smt::Op(smt::Extract,  right -> to_int(),  right -> to_int());

                        smt::Sort concat_sort = solver -> make_sort(smt::BV, current -> get_concat_width());
                        smt::Term concat_term = solver -> make_term(0, concat_sort);
                        terms[current] = solver -> make_term(smt::Concat, concat_term, solver -> make_term(extract_op, left));
                        break;
                    }
                    smt::Term left = terms[current->get_child().at(0)];
                    smt::Term right = terms[current->get_child().at(1)];
                    smt::Op extract_op = smt::Op(smt::Extract,  right -> to_int(),  right -> to_int());
                    terms[current] = solver -> make_term(extract_op, left);
                    break;
                }
                // range index [msb:lsb]
                case verilog_expr::voperator::RANGE_INDEX: {
                    if( (current -> get_concat_width()) != 0){
                        smt::Term left = terms[current->get_child().at(0)];
                        smt::Term mid = terms[current->get_child().at(1)];
                        smt::Term right = terms[current->get_child().at(2)];
                        smt::Op extract_op = smt::Op(smt::Extract,  mid -> to_int(),  right -> to_int());

                        smt::Sort concat_sort = solver -> make_sort(smt::BV, current -> get_concat_width());
                        smt::Term concat_term = solver -> make_term(0, concat_sort);
                        terms[current] = solver -> make_term(smt::Concat, concat_term, solver -> make_term(extract_op, left));
                        break;
                    }
                    smt::Term left = terms[current->get_child().at(0)];
                    smt::Term mid = terms[current->get_child().at(1)];
                    smt::Term right = terms[current->get_child().at(2)];
                    smt::Op extract_op = smt::Op(smt::Extract,  mid -> to_int(),  right -> to_int());
                    terms[current] = solver -> make_term(extract_op, left);
                    break;
                }
                // make var
                case verilog_expr::voperator::MK_VAR: {
                    if( (current -> get_concat_width()) != 0){
                        smt::Sort concat_sort = solver -> make_sort(smt::BV, current -> get_concat_width());
                        smt::Term concat_term = solver -> make_term(0, concat_sort);
                        terms[current] = solver -> make_term(smt::Concat, concat_term, sim.var(current->to_verilog()));
                        break;
                    }
                    terms[current] = sim.var(current->to_verilog());
                    break;
                }
                // make const
                case verilog_expr::voperator::MK_CONST: {
                    auto current_const = verilog_expr::VExprAstConstant::cast_ptr(current);
                    auto const_value_tuple = current_const -> get_constant();

                    auto base = std::get<0>(const_value_tuple);
                    auto width = std::get<1>(const_value_tuple);
                    auto value = std::get<2>(const_value_tuple);

                    int const_value;

                    switch(base){
                      case 0:{
                        width = 32;
                        const_value = std::stoi(value);
                        break;
                      }
                      case 2:{
                        const_value = std::stoi(value, nullptr, 2);
                        break;
                      }
                      case 10:{
                        const_value = std::stoi(value, nullptr, 10);
                        break;
                      }
                      case 16:{
                        const_value = std::stoi(value, nullptr, 16);
                        break;
                      }
                      default:{
                        const_value = 0;
                        break;
                      }
                    }

                    auto concat_width = current -> get_concat_width();
                    terms[current] = solver->make_term(const_value, solver->make_sort(smt::BV, width + concat_width));
                    break;
                }
                // undefined operation
                default:
                    throw std::runtime_error("Unknown operation in AST");
            }
        }
    }
    return terms[node];
}

//class TimedAssertionChecker
void TimedAssertionChecker::parse_verilog(const std::string& verilog_assertion){
  verilog_assertion_ = verilog_assertion;
  //deps from vexpparser
  Vexp::Interpreter intp;
  std::stringstream ss;
  ss << verilog_assertion;

  intp.switchInputStream(&ss);
  try{
    intp.parse();
  } catch (verilog_expr::VexpException &e) {
    std::cout << "AST constructor error:" << e.msg_ << std::endl;
  }

  ast = intp.GetAstRoot();
  std::cout << "vexpparser ast : " << ast << std::endl;
}

void TimedAssertionChecker::parse_ast(const verilog_expr::VExprAst::VExprAstPtr& ast){
    
    Variable var_struct;

    if (!ast) {
      throw std::invalid_argument("Null AST node");
    }

    std::stack<verilog_expr::VExprAst::VExprAstPtr> node_stack;
    node_stack.push(ast);

    while (!node_stack.empty()) {
        auto node = node_stack.top();
        node_stack.pop();

        switch (node->get_op()) {
            case verilog_expr::voperator::AT: {
                std::string fullname = {node -> get_child().at(0) -> to_verilog() + "@" + node -> get_child().at(1) -> to_verilog()};
                std::string name = node -> get_child().at(0) -> to_verilog();
                int cycle = std::stoi(node -> get_child().at(1) -> to_verilog());
                auto var_sort = sim_.var(node -> get_child().at(0) -> to_verilog()) -> get_sort();
                int width = var_sort -> get_width();

                var_struct = {fullname, name, cycle, width};
                var_struct_vec.push_back(var_struct);
                break;
            }
            default: {
                for (size_t i = 0; i < node -> get_child_cnt(); ++i) {
                    node_stack.push(node -> get_child().at(i));
                }
                break;
            }
        }
    }
}

void TimedAssertionChecker::parse_max_cycle()
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

void TimedAssertionChecker::sim_max_step(bool& rst_en0)
{
    int sim_cycle;
    const auto& vars = get_var_vec();
    
    //get input var for make vdict
    smt::UnorderedTermSet input_term_set = ts_.inputvars();
    smt::Term clk_term = sim_.var("clk");
    input_term_set.erase(clk_term);
    
    for (sim_cycle = 0; sim_cycle <= max_cycle; sim_cycle ++)
    {
        std::cout << "--------------------------------------@" << sim_cycle << std::endl;
        if(sim_cycle != 0)
        {
          sim_.sim_one_step();
        }
        
        
        //make vdict for each input sim.convert
        wasim::assignment_type input_vdict;
        for(auto &input_var : input_term_set){
          input_vdict[input_var -> to_string()] = input_var -> to_string() + std::to_string(sim_cycle);
        }

        if(rst_en0){
          input_vdict["rst"] = 0;
        }


        auto inputmap = sim_.convert(input_vdict); //input_vdict = {{"a", "a" + std::to_string(sim_cycle)}, {"b", "b" + std::to_string(sim_cycle)}}
        sim_.set_input(inputmap, {}); 

        auto s = sim_.get_curr_state();
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
            auto out_term = sim_.var(vars[j].name);  //get var term name
            ass_termmap[vars[j].fullname] = sim_.interpret_state_expr_on_curr_frame(out_term);
          }
        }
    }
}

void TimedAssertionChecker::print_term_map(){
  std::cout << "-----------------we got the symbolic term----------------" << std::endl;
  std::cout << "---------------------------------------------------------" << std::endl;
  std::cout << "term <-> symbolic term" << std::endl;
  for(const auto &var_term : ass_termmap)
  {
    std::cout << var_term.first << " <-> " << var_term.second << std::endl;
  }
  std::cout << "---------------------------------------------------------" << std::endl;
}

smt::UnorderedTermMap TimedAssertionChecker::create_substitution_map(){
  smt::UnorderedTermMap substitution_map;
  for(auto &var_struct : var_struct_vec){
    smt::Term origin_term = solver_ -> get_symbol(var_struct.fullname);
    smt::Term sub_term = ass_termmap[var_struct.fullname];
    substitution_map[origin_term] = sub_term;
  }
  return substitution_map;
}

void TimedAssertionChecker::make_assertion_term(){
  std::cout << "vexpparser AST : " << ast << std::endl;
    // make new ast tree (new version local width
  auto my_ast = get_ast();
  traverse_ast_and_set_extend_width(my_ast, sim_);
  std::cout << "AST set width extension : done" << std::endl;

    //ast transformer to term
  assertion_term = ast2term(solver_, my_ast, sim_); //my_ast(set width extension)
  std::cout << "my assertion term (original term) :" << assertion_term << std::endl;

    //create substitution map
  auto substitution_map = create_substitution_map();

    //substitude term
  assertion_term_sub =  solver_ -> substitute(assertion_term, substitution_map);
  std::cout << "my assertion term (substitude symbolic term) :" << assertion_term_sub << std::endl;
}

void TimedAssertionChecker::assert_formula(){
    //assert
  smt::Term not_assertion_term = solver_ -> make_term(smt::PrimOp::Not, assertion_term_sub);
  solver_ -> assert_formula(not_assertion_term);

  std::cout << "assert formula (not my assertion term) : done" << std::endl;
}

void TimedAssertionChecker::check_sat(){
    std::cout << "term check sat result : " << solver_ -> check_sat() << std::endl;
    std::cout << "---------------------------------------------------------" << std::endl;
    if( (solver_ -> check_sat()).result == smt::SAT )
    {
      std::cout << "X |"<< " assert(" + verilog_assertion_ + ") | the equation isn't always correct and has counter-example" << std::endl;
    }
    else if((solver_ -> check_sat()).result == smt::UNSAT )
    {
      std::cout << "√ |" << " assert(" + verilog_assertion_ + ") | the equation is always correct" << std::endl;
    }
}



}   //namespace tac