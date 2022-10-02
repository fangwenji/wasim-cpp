#include "term_manip.h"

namespace wasim{
    
smt::TermVec args(smt::Term term){
    smt::TermVec arg_vec;
    for (const auto& a : term){
        arg_vec.push_back(a);
    }
    return arg_vec;
}

smt::UnorderedTermSet get_free_variables(smt::Term term){
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

smt::Term free_make_symbol(std::string n, smt::Sort symb_sort, std::unordered_map<std::string, int>& name_cnt, smt::SmtSolver& solver){
    int cnt;
    if(name_cnt.find(n) == name_cnt.end()){
        cnt = 1;
    }
    else{
        auto cur_cnt = name_cnt[n];
        // const auto cur_cnt = iter->second;
        cnt = cur_cnt + 1; 
    }

    name_cnt[n] = cnt;
    smt::Term symb;
    try
    {
       symb =  solver->make_symbol(n + std::to_string(cnt), symb_sort);
       
    }
    catch(const std::exception& e)
    {
        name_cnt[n] = cnt + 1;
        symb = free_make_symbol(n, symb_sort, name_cnt, solver);
    }
    return symb;
}

} // namespace wasim
