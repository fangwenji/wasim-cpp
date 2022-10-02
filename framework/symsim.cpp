#include "symsim.h"

namespace wasim {


void ChoiceItem::setSim(){
    assert(not UsedInSim_);
    UsedInSim_ = true;
}

void ChoiceItem::CheckSimed(){
    assert(UsedInSim_);
}

void ChoiceItem::record_prev_assumption_len(int l){
    assmpt_len_ = l;
}

int ChoiceItem::get_prev_assumption_len(){
    return assmpt_len_;
}

int SymbolicExecutor::tracelen(){
    return trace_.size();
}

auto SymbolicExecutor::all_assumptions(){
    smt::TermVec ret_vec;
    for (auto l : history_assumptions_){
        for (auto c: l){
            ret_vec.push_back(c);
        }
    }
    return ret_vec;
}

auto SymbolicExecutor::all_assumption_interp(){
    std::vector<std::string> ret_vec;
    for (auto l : history_assumptions_interp_){
        for (auto c: l){
            ret_vec.push_back(c);
        }
    }
    return ret_vec;
}

smt::Term SymbolicExecutor::sv(std::string n){
    return ts_.named_terms_[n];
}

smt::Term SymbolicExecutor::cur(std::string n){
    auto sv_mapping = trace_.back();
    smt::Term expr = sv(n);
    if(not _expr_only_sv(expr)){
        assert(history_choice_.size() !=0);
        assert(not history_choice_.back().UsedInSim_);
        smt::UnorderedTermMap iv_mapping = history_choice_.back().var_assign_;
        auto subs_mapping = sv_mapping;
        subs_mapping.insert(iv_mapping.begin(), iv_mapping.end());
        smt::Term expr = solver_->substitute(expr, subs_mapping);
    }
    else{
        smt::Term expr = solver_->substitute(expr, sv_mapping);
    }
    return expr;
}

void SymbolicExecutor::_check_only_invar(smt::UnorderedTermMap vdict){
    for (auto v : vdict){
        assert(invar_.find(v.first) != invar_.end());
    }
}

bool SymbolicExecutor::_expr_only_sv(smt::Term expr){
    smt::UnorderedTermSet var_set = get_free_variables(expr);

    for (auto v : var_set){
        if (svar_.find(v) != svar_.end()){
            return true;
        }
        else{
            return false;
        }
    }
    exit(1);
}

smt::UnorderedTermMap SymbolicExecutor::convert(std::map<wasim::type_conv, wasim::type_conv> vdict){
    std::unordered_map<smt::Term, wasim::type_conv> retdict_temp;
    for (auto v : vdict){ // check key only   
        auto key = v.first;
        auto value = v.second;
        smt::Term key_new;
        if (std::holds_alternative<std::string>(key)){
            auto key_str = std::get<std::string>(key);
            key_new = sv(key_str);
            retdict_temp[key_new] = value;
        }
    }
    smt::UnorderedTermMap retdict;
    for (auto v : retdict_temp){
        auto key = v.first;
        auto value = v.second;
        if (std::holds_alternative<std::string>(value)){
            auto value_str = std::get<std::string>(value);
            auto key_sort = key->get_sort();
            auto value_new = solver_->make_symbol(value_str, key_sort);
            retdict[key] = value_new;
        }
        else if (std::holds_alternative<int>(value)){
            auto value_int = std::get<int>(value);
            auto key_sort = key->get_sort();
            auto key_width = key_sort->get_width();
            auto value_sort = solver_->make_sort(smt::BV, key_width);
            // auto value_new = solver_->make_symbol(value_sort->to_string(),value_sort);
            auto value_new = solver_->make_term(value_int, value_sort);
            retdict[key] = value_new;
        }

    }
    return retdict;
}

void SymbolicExecutor::backtrack(){
    assert(history_choice_.size() != 0);
    trace_.pop_back();
    history_assumptions_.pop_back();
    history_assumptions_interp_.pop_back();
    history_choice_.back().UsedInSim_ = false;
}

void SymbolicExecutor::init(smt::UnorderedTermMap var_assignment /*={}*/){
    for (auto v : svar_){
        if (var_assignment.find(v) == var_assignment.end()){
            var_assignment[v] = new_var(v->get_sort()->get_width(), v->to_string(), false);
        }
    }
    trace_.push_back(var_assignment);


    _expr_only_sv(ts_.init_);

    auto init_constr = solver_->substitute(ts_.init_,var_assignment);
    std::vector<smt::Term> init_vect;
    std::vector<std::string> init_vect_interp;
    init_vect.push_back(init_constr);
    history_assumptions_.push_back(init_vect);
    init_vect_interp.push_back("init");
    history_assumptions_interp_.push_back(init_vect_interp);
}

void SymbolicExecutor::set_current_state(StateAsmpt s, smt::UnorderedTermMap d){
    trace_.clear();
    smt::UnorderedTermMap sv_copy (s.sv_);
    sv_copy.insert(d.begin(), d.end());
    // TODO: copy problem
    auto var_assignment (sv_copy);

    trace_.push_back(var_assignment);

    // smt::TermVec asmpt_copy(s.asmpt_.begin(), s.asmpt_.end());
    // std::vector<std::string> asmpt_interp_copy(s.assumption_interp_.begin(), s.assumption_interp_.end());
    auto asmpt_copy (s.asmpt_);
    auto asmpt_interp_copy (s.assumption_interp_);
    history_assumptions_.clear();
    history_assumptions_.push_back(asmpt_copy);
    history_assumptions_interp_.clear();
    history_assumptions_interp_.push_back(asmpt_interp_copy);
    history_choice_.clear();
}

void SymbolicExecutor::print_current_step(){
    const auto& prev_sv = trace_.back();
    // cout << "sv  rhs" << endl;
    cout << "--------------------------------" << endl;
    cout  << "| " << setiosflags(ios::left) << setw(20) << "sv"  << "| " << setw(20) << "value" << endl;
    cout << "--------------------------------" << endl;
    for (const auto& sv : prev_sv){
        cout << "| "  << setiosflags(ios::left) << setw(20) << sv.first->to_string() 
        << "| "  << setw(20) << sv.second->to_string() << endl;
    }
}

void SymbolicExecutor::print_current_step_assumptions(){
    int i=0;
    for(const auto& l:history_assumptions_){
        int j=0;
        for(const auto& a:l)
        {
            
            const auto& interp = history_assumptions_interp_.at(i).at(j);
            cout << "A" << i << ", "<< j << " " << interp << endl;
            cout << "A" << i << ", "<< j << " " << a << endl;
            j++;
            
        }
        i++;
    }
}

void SymbolicExecutor::set_input(smt::UnorderedTermMap invar_assign, const smt::TermVec pre_assumptions){
    if (history_choice_.size() !=0){
        history_choice_.back().CheckSimed();
    }



    _check_only_invar(invar_assign);
    auto prev_sv (trace_.back());
    for(auto v : invar_){
         if (prev_sv.find(v) != prev_sv.end()){
            cout << "WARNING: ignore input assignment as assigned by prev-state " << v->to_string() << endl;
         }
         if (invar_assign.find(v) == invar_assign.end()){
            invar_assign[v] = new_var(v->get_sort()->get_width(), v->to_string(), true);
         }
    }
    ChoiceItem c(pre_assumptions, invar_assign);
    
    int len = static_cast<int> (history_assumptions_.back().size());
    c.record_prev_assumption_len(len);
    history_choice_.push_back(c);
    assert(history_assumptions_.size() == history_assumptions_interp_.size());
    assert(history_assumptions_.back().size() == history_assumptions_interp_.back().size());

    auto submap (prev_sv); // copy problem?
    submap.insert(invar_assign.begin(), invar_assign.end());

    // TODO: constraints here are different from those in COSA btor parser! (important)
    smt::TermVec assmpt_vec;
    for(const auto& vect : ts_.constraints_){
        auto vect_fir_subs = solver_->substitute(vect.first, submap);
        auto vect_sec_subs = solver_->make_term(vect.second);
        auto assmpt_temp = solver_->make_term(smt::Equal,vect_fir_subs, vect_sec_subs);
        assmpt_vec.push_back(assmpt_temp); 
    }
    smt::Term assmpt;
    if(assmpt_vec.size() == 1){
        assmpt = assmpt_vec.back();
    }
    else{
        assmpt = solver_->make_term(smt::And, assmpt_vec);
    }
    
    // for(const auto& asmpt : assmpt_vec){
    //     assmpt = solver_->make_term(smt::And,assmpt,asmpt);
    // }
    
    history_assumptions_.back().push_back(assmpt);
    history_assumptions_interp_.back().push_back("ts.asmpt @" + (std::to_string(trace_.size()-1)));
    
    
    for (auto vect : pre_assumptions){
        auto assmpt_temp = solver_->substitute(vect, submap);
        history_assumptions_.back().push_back(assmpt_temp);
        history_assumptions_interp_.back().push_back(assmpt_temp->to_string() + "@" + (std::to_string(trace_.size()-1)));
    }
}

void SymbolicExecutor::undo_set_input(){
    assert(history_choice_.size() != 0);
    auto c = history_choice_.back();
    assert(not c.UsedInSim_);
    history_choice_.pop_back();
    auto l = c.get_prev_assumption_len();
    smt::TermVec::const_iterator asmpt_fir = history_assumptions_.back().begin();
    smt::TermVec::const_iterator asmpt_sec = history_assumptions_.back().begin() + l;
    history_assumptions_.back().assign(asmpt_fir, asmpt_sec);
    auto asmpt_interp_fir = history_assumptions_interp_.back().begin();
    auto asmpt_interp_sec = history_assumptions_interp_.back().begin() + l;
    history_assumptions_interp_.back().assign(asmpt_interp_fir, asmpt_interp_sec);
}

std::any SymbolicExecutor::interpret_state_expr_on_curr_frame(std::any expr){
    auto prev_sv = trace_.back();
    if(expr.type() == typeid(smt::TermVec)){
        smt::TermVec expr_list = std::any_cast<smt::TermVec>(expr);
        smt::TermVec ret;
        for (auto e : expr_list){
            smt::Term e_term = std::any_cast<smt::Term>(interpret_state_expr_on_curr_frame(e));
            ret.push_back(e_term);
            return ret;
        }
    }
    else{
        smt::Term expr = std::any_cast<smt::Term>(expr);
        if(not _expr_only_sv(expr)){
            cout << "expr should only contain only state variables" << endl;
            exit(1);
            return NULL;
        }
        return solver_->substitute(expr, prev_sv);
    }
    
    exit(1);
}

void SymbolicExecutor::sim_one_step(){
    assert(history_choice_.size() != 0);

    auto& c = history_choice_.back();
    c.setSim();

    const auto& invar_assign = c.var_assign_;
    const auto& prev_sv = trace_.back();
    smt::UnorderedTermMap svmap;
    auto submap = prev_sv; 
    submap.insert(invar_assign.begin(), invar_assign.end());
    for(auto sv : ts_.state_updates_){
        svmap[sv.first] = solver_->substitute(sv.second, submap);
    }
    trace_.push_back(svmap);
    history_assumptions_.push_back({});
    history_assumptions_interp_.push_back({});
}

void SymbolicExecutor::sim_one_step_direct(){
    auto prev_sv = trace_.back();
    smt::UnorderedTermMap svmap;
    auto submap = prev_sv; 
    for(auto sv : ts_.state_updates_){
        svmap[sv.first] = solver_->substitute(sv.second, submap);
    }
    trace_.push_back(svmap);
    history_assumptions_.push_back({});
    history_assumptions_interp_.push_back({});
}

smt::UnorderedTermSet SymbolicExecutor::get_Xs(){
    return Xvar_;
}

smt::Term SymbolicExecutor::new_var(int bitwdth, std::string vname /*"=var"*/, bool x /*=true*/){
    smt::Term symb;
    std::string n;
    if(x){
        n = vname + "X";
    }
    else{
        n = vname;
    }
    // int cnt;
    // if(name_cnt_.find(n) == name_cnt_.end()){
    //     cnt = 1;
    // }
    // else{
    //     auto cur_cnt = name_cnt_[n];
    //     // const auto cur_cnt = iter->second;
    //     cnt = cur_cnt + 1; 
    // }
    // name_cnt_[n] = cnt;
    auto symb_sort = solver_->make_sort(smt::BV, bitwdth);

    symb = free_make_symbol(n, symb_sort, name_cnt_, solver_);

    
    // srand((unsigned int)(time (NULL)));
    // int rand_suf = rand();
    // symb = solver_->make_symbol(n + std::to_string(cnt+rand_suf) , symb_sort);
    // while (solver_->get_symbol(n + std::to_string(cnt)) != NULL)
    // {
    //     cnt++;
    // }
    // symb = solver_->make_symbol(n + std::to_string(cnt + 1), symb_sort);
    // name_cnt_[n] = cnt;

    // try
    // {
    //     symb = solver_->make_symbol(n + std::to_string(cnt), symb_sort);
    // }
    // catch(const std::exception& e)
    // {
    //     // auto cnt_new = cnt + rand_suf;
    //     symb = solver_->get_symbol(n + std::to_string(cnt));
    //     // symb = solver_->make_symbol(n + std::to_string(cnt_new), symb_sort);
    //     name_cnt_[n] = cnt + 1;
        
    // }
    // symb = solver_->make_symbol(n + std::to_string(cnt), symb_sort);
    
    if(x){
        Xvar_.insert(symb);
    }
    return symb;
}

StateAsmpt SymbolicExecutor::get_curr_state(smt::TermVec assumptions /*= {}*/){
    auto need_to_push_input = false;
    if((history_choice_.size() == 0) or (history_choice_.back().UsedInSim_)){
        need_to_push_input = true;
    }
    if(need_to_push_input){
        set_input({}, assumptions);
    }
    else{
        if(assumptions.size() != 0){
            cout << "WARNING: assumptions are not used in get_curr_state" << endl;
        }
    }
    StateAsmpt ret(trace_.back(), all_assumptions(), all_assumption_interp());
    if(need_to_push_input){
        undo_set_input();
    }
    return ret;
}

auto SymbolicExecutor::set_var(int bitwdth, std::string vname /*= "var"*/){
    auto symb_sort = solver_->make_sort(smt::BV, bitwdth);
    auto symb = solver_->make_symbol(vname, symb_sort);
    return symb;
}
}