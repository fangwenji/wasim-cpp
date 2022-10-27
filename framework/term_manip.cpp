#include "term_manip.h"

namespace wasim{
    
smt::TermVec args(const smt::Term & term){
    smt::TermVec arg_vec;
    for(auto pos = term->begin(); pos != term->end(); ++pos)
        arg_vec.push_back(*pos);
    
    return arg_vec;
}

smt::UnorderedTermSet get_free_variables(const smt::Term & term){
    smt::UnorderedTermSet free_var;
    smt::TermVec search_stack;
    if(term->is_symbol()){
        free_var.insert(term);
    }
    else{ // DFS
        search_stack.push_back(term);
        while (search_stack.size() != 0)
        {
            auto node = search_stack.back();
            search_stack.pop_back();
            if(node->is_symbol()){ // check
                free_var.insert(node);
            }
            auto children_vec = args(node);
            std::reverse(std::begin(children_vec), std::end(children_vec));
            search_stack.insert(search_stack.end(), children_vec.begin(), children_vec.end());
        }
        
    }
    return free_var;
}

smt::Term free_make_symbol(const std::string & n, smt::Sort symb_sort, std::unordered_map<std::string, int>& name_cnt, smt::SmtSolver& solver){
    int cnt;
    if(name_cnt.find(n) == name_cnt.end())
        cnt = 0;
    else
        cnt = name_cnt[n];
    
    do{
        ++cnt;
        name_cnt[n] = cnt;
        smt::Term symb;
        try
        {
            symb =  solver->make_symbol(n + std::to_string(cnt), symb_sort);
            return symb;
        }
        catch(const std::exception & e) {   // maybe name conflict
            std::cout << "New symbol: " << n + std::to_string(cnt) << " failed." << std::endl;
        }
    } while(true);
}

PropertyInterface::PropertyInterface(std::string filename, smt::UnorderedTermMap assign_map, smt::SmtSolver &solver)
    : super(solver), filename_(filename), assign_map_(assign_map), solver_(solver)
{
  set_logic_all();
  int res = parse(filename_);
  assert(!res);  // 0 means success

  for(const auto & n_prop : defs_){
      if(n_prop.first.find("assertion.") == 0)
        assertions_.push_back(n_prop.second);
      if(n_prop.first.find("assumption.") == 0)
        assumptions_.push_back(n_prop.second);
  }
}


smt::Term PropertyInterface::register_arg(const std::string & name, const smt::Sort & sort) {
//   auto tmpvar = ts_.lookup(name);

  cout << name << endl;
  cout << sort->to_string() << endl;

  smt::UnorderedTermSet assign_set;
  for(const auto& map: assign_map_){
    assign_set.insert(map.second);
  }
  
  smt::Term tmpvar;
  for(const auto& symvar: assign_set){
    if(name == symvar->to_string()){
        tmpvar = symvar;
        cout << name << endl;
    }
  }

  

//   auto tmpvar = assign_set.find(name);
//   if (it == named_terms_.end()) {
//     throw PonoException("Could not find term named: " + name);
//   }
//   return it->second;
  arg_param_map_.add_mapping(name, tmpvar);
  return tmpvar; // we expect to get the term in the transition system.

  // TODO: symbolic values do not exist in the transition system
}

smt::Term PropertyInterface::return_defs() {
    for(const auto &d : defs_){
        cout << d.first << " : " << d.second << endl;
        return d.second;
    }

}
} // namespace wasim
