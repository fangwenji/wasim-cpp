#include "term_manip.h"

namespace wasim{
    
smt::TermVec arg(smt::Term term){
    smt::TermVec arg_vec;
    for (const auto& a : term){
        arg_vec.push_back(a);
    }
    return arg_vec;
}

smt::UnorderedTermSet get_free_variable(smt::Term term){
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
            auto children_vec = arg(node);
            std::reverse(std::begin(children_vec), std::end(children_vec));
            search_stack.insert(search_stack.end(), children_vec.begin(), children_vec.end());
        }
        
    }
    return free_var;
}


} // namespace wasim
