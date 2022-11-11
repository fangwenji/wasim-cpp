#include "sygus_simplify.h"

namespace wasim{

void run_cmd(std::string cmd_string, int timeout){
    boost::process::child c(cmd_string);
    std::error_code ec;
    if (!c.wait_for(std::chrono::milliseconds(timeout), ec)) {
        // std::cout << "nTimeout reached. Process terminated after " << timeout << " milliseconds.\n";
        // exit(1);
        c.terminate(ec);
    }
}

bool expr_contians_X(smt::Term expr, smt::UnorderedTermSet set_of_xvar){
    auto vars_in_expr = get_free_variables(expr);
    for (const auto & var : vars_in_expr){
        if(set_of_xvar.find(var) != set_of_xvar.end()){
            return true;
        }
    }
    return false;
}


std::string GetTimeStamp()
{
	auto timeNow = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
	long long timestamp = timeNow.count();
	return std::to_string(timestamp);
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

smt::Term run_sygus(parsed_info info, smt::UnorderedTermSet set_of_xvar, smt::SmtSolver& solver){
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
    auto bash_file = "sygus_" + time_stamp + ".sh";

    std::ofstream f;

    f.open(template_file.c_str(), ios::out | ios::app);
    auto line1 = "(set-logic BV)\n\n\n(synth-fun FunNew \n   (";
    f << line1 << endl;

    // for(auto xvar:set_of_xvar){
    //     cout << xvar->to_string();
    // }
    // cout << endl;
    for(const auto& var : free_var){
        if(set_of_xvar.find(var) == set_of_xvar.end()){
            auto line2 = "    (" + var->to_string() + " " + var->get_sort()->to_string() + " )";
            f << line2 << endl;
        }
    }

    auto line3 = "   )\n   " + Fun_type + "\n  )\n\n\n";
    f << line3 << endl;

    for(const auto& var : free_var_asmpt){
            auto line4 = "(declare-var " + var->to_string() + " " + var->get_sort()->to_string() + ")";
            f << line4 << endl;
    }

    for(const auto& var : free_var){
        if(free_var_asmpt.find(var) == free_var_asmpt.end()){
            auto line4 = "(declare-var " + var->to_string() + " " + var->get_sort()->to_string() + ")";
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
            auto line8 = var->to_string()+" ";
            f << line8;
        }
    }

    auto line9 = ") ;\n    )))\n\n\n;\n\n(check-synth)";
    f << line9 << endl;
    f.close();

    std::ofstream bash_f;

    bash_f.open(bash_file.c_str(), ios::out | ios::app);
    auto bash_line1 = "#!/bin/bash";
    bash_f << bash_line1 << endl;
    auto bash_line2 = "/data/wenjifang/cvc5-Linux --lang=sygus2 " + template_file + " > " + result_temp_file;
    bash_f << bash_line2 << endl;
    bash_f.close();
    
    auto cmd = "chmod 755 " + bash_file;
    system(cmd.c_str());
    auto cmd_string = "./" + bash_file;
    run_cmd(cmd_string, 500); // 500 milliseconds -> 0.5s

    // only move the second line of sygus output file to a new file
    std::ifstream infile(result_temp_file.c_str());
    std::string linedata = "";

    getline(infile, linedata); // get first line
    getline(infile, linedata); // get second line
    infile.close();

    std::ofstream outfile;
    outfile.open(result_file.c_str(), ios::out | ios::app);
    outfile << linedata << endl;
    outfile.close();

    smt::Term new_expr;
    // determine whether the smtlib_result is empty
    if(linedata.empty()){
        try {
            new_expr = solver->make_symbol("no_file",solver->make_sort(smt::BV, 1));
        }
        catch (const std::exception & e) {
            new_expr = solver->get_symbol("no_file");
        }
    }
    else{
        PropertyInterface pi(result_file, solver);
        new_expr = pi.return_defs();
    }
    int rm;
    rm = remove(template_file.c_str());
    rm = remove(result_file.c_str());
    rm = remove(result_temp_file.c_str());
    rm = remove(bash_file.c_str());

    return new_expr;
}

smt::TermVec child_vec_simplify(smt::TermVec child_vec, StateAsmpt state, smt::UnorderedTermSet set_of_xvar, smt::SmtSolver& solver, smt::SmtSolver& solver_cvc5){
    smt::TermVec child_new_vec = {};
    for(auto child : child_vec){
        smt::Term child_new;
        if(expr_contians_X(child, set_of_xvar)){
            child_new = structure_simplify(child, state, set_of_xvar, solver, solver_cvc5);
        }
        else{
            child_new = child;
        }
        child_new_vec.push_back(child_new);
    }
    return child_new_vec;
}

smt::Term structure_simplify(smt::Term v, StateAsmpt state, smt::UnorderedTermSet set_of_xvar, smt::SmtSolver& solver, smt::SmtSolver& solver_cvc5){
    

    smt::Term new_expr;
    auto expr_info = parse_state(state, v, solver_cvc5);
    auto new_expr_direct = run_sygus(expr_info, set_of_xvar, solver_cvc5);
    if(new_expr_direct->to_string() == "no_file"){
        auto child_vec = args(v);
        auto child_new_vec = child_vec_simplify(child_vec, state, set_of_xvar, solver, solver_cvc5);
        if((v->get_op() == smt::Ite) && (child_vec.size() == 3)){
            new_expr = solver_cvc5->make_term(smt::Ite, child_new_vec);
        }
        else if((v->get_op() == smt::BVNot) && (child_vec.size() == 1)){
            new_expr = solver_cvc5->make_term(smt::BVNot, child_new_vec);
        }
        else if((v->get_op() == smt::Not) && (child_vec.size() == 1)){
            new_expr = solver_cvc5->make_term(smt::Not, child_new_vec);
        }
        else if((v->get_op() == smt::BVAnd) && (child_vec.size() == 2)){
            new_expr = solver_cvc5->make_term(smt::BVAnd, child_new_vec);
        }
        else if((v->get_op() == smt::And) && (child_vec.size() == 2)){
            new_expr = solver_cvc5->make_term(smt::And, child_new_vec);
        }
        else if((v->get_op() == smt::BVMul) && (child_vec.size() == 2)){
            new_expr = solver_cvc5->make_term(smt::BVMul, child_new_vec);
        }
        else if((v->get_op() == smt::BVAdd) && (child_vec.size() == 2)){
            new_expr = solver_cvc5->make_term(smt::BVAdd, child_new_vec);
        }
        else if((v->get_op() == smt::Concat) && (child_vec.size() == 2)){
            new_expr = solver_cvc5->make_term(smt::Concat, child_new_vec);
        }
        else if((v->get_op() == smt::Equal) && (child_vec.size() == 2)){
            new_expr = solver_cvc5->make_term(smt::Equal, child_new_vec);
        }
        // else if
        else{
            cout << "new structure?\n" << v->to_string() << endl;
            cout << "node op: " << v->get_op() << endl;
            cout << "num of args: " << child_vec.size() << endl;
            exit(1);
        }
    }
    else{
        new_expr = new_expr_direct;
    }

    return new_expr;
}

void sygus_simplify(StateAsmpt& state_btor, smt::UnorderedTermSet set_of_xvar_btor, smt::SmtSolver& solver){
    smt::SmtSolver solver_cvc5 = smt::Cvc5SolverFactory::create(false);
    solver_cvc5->set_logic("QF_BV");
    solver_cvc5->set_opt("produce-models", "true");
    solver_cvc5->set_opt("incremental", "true");
    for(auto sv : state_btor.sv_){
        auto s_btor = sv.first;
        auto v_btor = sv.second;
        if (expr_contians_X(v_btor, set_of_xvar_btor)){
            auto v = TermTransfer(v_btor, solver, solver_cvc5);
            auto state = StateTransfer(state_btor, solver, solver_cvc5);
            auto set_of_xvar = SetTransfer(set_of_xvar_btor, solver, solver_cvc5);
            auto new_expr = structure_simplify(v, state, set_of_xvar, solver, solver_cvc5);
            // cout << "new_expr: " << new_expr->to_string() << endl;
            auto new_expr_btor = TermTransfer(new_expr, solver_cvc5, solver);
            state_btor.sv_.insert_or_assign(s_btor, new_expr_btor);
        }
    }
}


}