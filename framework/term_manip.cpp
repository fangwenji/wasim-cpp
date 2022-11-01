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
            // std::cout << "New symbol: " << n + std::to_string(cnt) << " failed." << std::endl;
            
        }
    } while(true);
}

PropertyInterface::PropertyInterface(std::string filename, smt::SmtSolver &solver)
    : super(solver), filename_(filename), solver_(solver)
{
  set_logic_all();
  int res = parse(filename_);
  assert(!res);  // 0 means success
}


smt::Term PropertyInterface::register_arg(const std::string & name, const smt::Sort & sort) {

  smt::Term tmpvar;
  try {
            tmpvar = solver_->get_symbol(name);
        } catch(const std::exception& e) {
            cout << "ERROR: Could not find " << name << " in solver! Wrong input value."<< endl;
        }
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
