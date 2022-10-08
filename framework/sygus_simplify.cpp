#include "sygus_simplify.h"

namespace wasim{

void run_cmd(std::string cmd_string, int timeout){
    boost::process::child c(cmd_string);
    std::error_code ec;
    if (!c.wait_for(std::chrono::seconds(timeout), ec)) {
        std::cout << "nTimeout reached. Process terminated after "
                << timeout << " seconds.\n";
        c.terminate(ec);
    }
}

bool expr_contians_X(smt::Term expr, smt::UnorderedTermSet set_of_xvar){
    auto vars_in_expr = get_free_variables(expr);
    for (const auto & var : vars_in_expr){
        if(set_of_xvar.find(var) == set_of_xvar.end()){
            return false;
        }
    }
    return true;
}


std::string GetTimeStamp()
{
	auto timeNow = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
	long long timestamp = timeNow.count();
	return std::to_string(timestamp);
}


smt::Term structure_simplify(smt::Term v, StateAsmpt state, smt::UnorderedTermSet set_of_xvar, smt::SmtSolver& solver){
    auto child_vec = args(v);
    auto child_new_vec = child_vec_simplify(child_vec, state, set_of_xvar);
    if((v->get_op() == smt::Ite) and (child_vec.size() == 3)){
        auto new_expr = solver->make_term(smt::Ite, child_new_vec);
    }
    // else if
    else{

    }
}


parsed_info parse_state(StateAsmpt state, smt::Term v, smt::SmtSolver& solver){
    auto asmpt_and = solver->make_term(smt::And, state.asmpt_);
    auto free_var_asmpt = get_free_variables(asmpt_and);
    auto Fun = v;
    auto free_var = get_free_variables(v);
    auto Fun_type = v->get_sort()->to_string();
    auto ret = std::make_tuple(free_var, free_var_asmpt, asmpt_and, Fun, Fun_type);

    return ret;
}



void sygus_simplify(StateAsmpt state, smt::UnorderedTermSet set_of_xvar, smt::SmtSolver& solver){
    for(auto sv : state.sv_){
        auto s = sv.first;
        auto v = sv.second;
        if (expr_contians_X(v, set_of_xvar)){
            auto new_expr = structure_simplify(v, state, set_of_xvar, solver);
            // smt::UnorderedTermMap new_sv_map = {s, new_expr};
            state.sv_.insert_or_assign(s, new_expr);
        }
    }
}

smt::Term run_sygus(parsed_info info, smt::UnorderedTermSet set_of_xvar){
    smt::UnorderedTermSet free_var;
    smt::UnorderedTermSet free_var_asmpt;
    smt::Term asmpt_and;
    smt::Term Fun;
    std::string Fun_type;
    tie(free_var, free_var_asmpt, asmpt_and, Fun, Fun_type) = info;
    auto time_stamp = GetTimeStamp();
    auto template_file = "sygus_template_" + time_stamp + ".sygus";
    auto result_file = "sygus_result_" + time_stamp + ".sygus";
    auto result_temp_file = "sygus_result_temp_" + time_stamp + ".sygus";

    ofstream f;

    f.open(template_file.c_str(), ios::out | ios::app);
    auto line1 = "(set-logic BV)\n\n\n(synth-fun FunNew \n   (";
    f << line1 << endl;

    for(const auto& var : free_var){
        if(set_of_xvar.find(var) == set_of_xvar.end()){
            auto line2 = "    (" + var->to_string() + " " + var->get_sort()->to_string() + " )";
            f << line2 << endl;
        }
    }

    auto line3 = "   )\n   " + Fun_type + "\n  )\n\n\n";
    f << line3 << endl;

    for(const auto& var : free_var_asmpt){
            auto line4 = "(declare-var " + var->to_string() + var->get_sort()->to_string() + ")";
            f << line4 << endl;
    }

    for(const auto& var : free_var){
        if(free_var_asmpt.find(var) == free_var_asmpt.end()){
            auto line4 = "(declare-var " + var->to_string() + var->get_sort()->to_string() + ")";
            f << line4 << endl;
        }      
    }

    auto line5 = "\n(constraint (=> ";
    f << line5 << endl;

    auto line6 = "    " + asmpt_and->to_string() + " ;\n\n    (=";
    f << line6 << endl;

    auto line7 = "        " + Fun->to_string() +" ;\n        (FunNew ";
    f << line7;

    for(const auto& var : free_var){
        if(set_of_xvar.find(var) == set_of_xvar.end()){
            auto line8 = var->to_string();
            f << line8;
        }
    }

    auto line9 = ") ;\n    )))\n\n\n;\n\n(check-synth)";
    f << line9 << endl;
    f.close();

    auto cmd_string = "/data/wenjifang/cvc5-Linux --lang=sygus2 " + template_file + " > " + result_temp_file;
    run_cmd(cmd_string, 0.5);

}

}