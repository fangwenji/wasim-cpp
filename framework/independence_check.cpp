#include"independence_check.h"
namespace wasim {
//TODO: need to distinguish constant and op?
bool e_is_always_valid(smt::Term e, smt::TermVec assumptions /*={}*/, smt::SmtSolver s){
    bool is_bool = (e->get_sort()->get_sort_kind() == smt::BOOL);
    bool is_bv = (e->get_sort()->get_sort_kind() == smt::BV);

    if(is_bool){
        auto not_e = s->make_term(smt::Not, e);
        assumptions.push_back(not_e);
    }
    else if(is_bv){
        if(e->get_sort()->get_width() == 1){
            auto sort_0 = s->make_sort(smt::BV, 1);
            auto bv_0 = s->make_term(0, sort_0);
            auto eq_e = s->make_term(smt::Equal, e, bv_0);
            assumptions.push_back(eq_e);
        }
    }

    return not(s->check_sat_assuming(assumptions).is_sat());
}

bool e_is_always_invalid(smt::Term e, smt::TermVec assumptions /*={}*/, smt::SmtSolver s){
    bool is_bool = (e->get_sort()->get_sort_kind() == smt::BOOL);
    bool is_bv = (e->get_sort()->get_sort_kind() == smt::BV);

    if(is_bool){
        assumptions.push_back(e);
    }
    else if(is_bv){
        if(e->get_sort()->get_width() == 1){
            auto sort_1 = s->make_sort(smt::BV, 1);
            auto bv_1 = s->make_term(1, sort_1);
            auto eq_e = s->make_term(smt::Equal, e, bv_1);
            assumptions.push_back(eq_e);
        }
    }

    return not(s->check_sat_assuming(assumptions).is_sat());
}

bool e_is_independent_of_v(smt::Term e, smt::Term v, smt::TermVec assumptions /*={}*/, smt::SmtSolver s){
    auto w = v->get_sort()->get_width();
    auto sort_w = s->make_sort(smt::BV, w);
    auto v1 = s->make_symbol(v->to_string()+"1", sort_w);
    auto v2 = s->make_symbol(v->to_string()+"2", sort_w);

    smt::UnorderedTermMap sub_map1, sub_map2;
    sub_map1 = {{v,v1}};
    sub_map2 = {{v,v2}};
    auto e1 = s->substitute(e, sub_map1);
    auto e2 = s->substitute(e, sub_map2);

    smt::TermVec assumptionSub;
    auto assumptionSub_expr = s->make_term(smt::Not, s->make_term(smt::Equal, e1, e2));
    cout << "assump: " << assumptionSub_expr << endl;
    assumptionSub.push_back(assumptionSub_expr);
    for (auto a : assumptions){
        assumptionSub.push_back(s->substitute(a, sub_map1));
        assumptionSub.push_back(s->substitute(a, sub_map2));
    }


    auto r = s->check_sat_assuming(assumptionSub);



    auto ret = not(r.is_sat());
    return ret;
}

smt::Term substitute_simplify(smt::Term e, smt::Term v, smt::TermVec assumptions /*={}*/, smt::SmtSolver s){
    smt::Term val;
    if(assumptions.size() == 0){
        auto v_sort = v->get_sort();
        val = s->make_term(0, v_sort);
    }
    else{ //this part need to test
        auto asmpt_term = s->make_term(smt::And, assumptions);
        s->assert_formula(asmpt_term);
        auto val = s->get_value(v);
        s->reset_assertions();
    }
    smt::UnorderedTermMap submap = {{v,val}};
    return s->substitute(e, submap);
}

bool is_valid(smt::Term e, smt::SmtSolver s){
    smt::TermVec e_vec;
    e_vec.push_back(e);
    return s->check_sat_assuming(e_vec).is_sat();
}

}
    