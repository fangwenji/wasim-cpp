#include <framework/state_read_write.h>

#include "smt-switch/utils.h"

namespace wasim {

#error "remember to use the sort of variables"
void StateRW::write_term(const std::string & outfile, const smt::Term & expr)
{
  smt::UnorderedTermSet free_value_var;
  smt::get_free_symbols(expr, free_value_var);
  std::ofstream f;
  f.open(outfile.c_str(), ios::out | ios::app);
  auto line_start = "(define-fun FunNew (";
  auto line_end = ")";
  auto expr_sort = expr->get_sort()->to_string();
  auto expr_name = expr->to_string();
  auto v_sort = expr->get_sort()->to_string();
  // cout << expr->to_string() << endl;
  f << line_start;
  for (const auto & v : free_value_var) {
    f << "(" + v->to_string() + " " + v_sort + ") ";
  }
  f << line_end;
  f << " " + expr_sort + " ";
  f << expr_name + line_end << endl;
}

void StateRW::write_single_term(const std::string & outfile, const smt::Term & expr)
{
  std::ofstream f;
  f.open(outfile.c_str(), ios::out | ios::app);
  auto line_start = "(define-fun FunNew (";
  auto line_end = ")";
  auto expr_sort = expr->get_sort()->to_string();
  auto expr_name = expr->to_string();
  if (is_bv_const(expr)) {
    f << expr_name << endl;
  } else {
    f << line_start;
    f << "(" + expr_name + " " + expr_sort + ")";
    f << line_end;
    f << " " + expr_sort + " ";
    f << "(ite (= #b1 #b1) " + expr_name + " " + expr_name + ")" + line_end
      << endl;
  }
}

void StateRW::write_string(const std::string & outfile, const std::string & asmpt_interp)
{
  std::ofstream f;
  f.open(outfile.c_str(), ios::out | ios::app);
  f << asmpt_interp << endl;
}

bool StateRW::is_bv_const(const smt::Term & expr)
{
  if (expr->get_sort()->get_sort_kind() == smt::BOOL) {
    return true;
  } else if (expr->get_sort()->get_sort_kind() == smt::BV) {
    if (expr->is_value()) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

smt::Term StateRW::str2bvnum(const std::string & int_num)
{
  auto str_num = int_num.substr(2);
  int bitwidth = static_cast<int>(str_num.size());
  int ret_dex = 0;
  for (int i = 0; i < str_num.size(); ++i) {
    if (str_num.at(i) == '1') {
      ret_dex += pow(2.0, bitwidth - i - 1);
    }
  }
  auto ret_term =
      solver_->make_term(ret_dex, solver_->make_sort(smt::BV, bitwidth));

  return ret_term;
}

void StateRW::StateWrite(const wasim::StateAsmpt & state,
                         const std::string & outfile_sv,
                         const std::string & outfile_asmpt)
{
  // 1. write state variables and expressions to outfile_sv
  for (const auto & sv : state.get_sv()) {
    // 1.1 for variable (LHS)
    const auto & var = sv.first;
    write_single_term(outfile_sv, var);

    // 1.2 for value expression (RHs)
    const auto & value = sv.second;
    smt::UnorderedTermSet free_value_var;
    smt::get_free_symbols(value, free_value_var);
    if (is_bv_const(value)) {
      write_single_term(outfile_sv, value);
    } else if ((free_value_var.size() == 1) && (value->get_op().is_null())) {
      write_single_term(outfile_sv, value);
    } else {
      write_term(outfile_sv, value);
    }
  }
  // 2. write assumptions and interpretations to outfile_asmpt
  for (int i = 0; i < state.get_assumptions().size(); i++) {
    const auto & asmpt = state.get_assumptions().at(i);
    smt::UnorderedTermSet free_asmpt_var;
    smt::get_free_symbols(asmpt, free_asmpt_var);
    #error "fix me"
    if (is_bv_const(asmpt)) {
      write_single_term(outfile_asmpt, asmpt);
    } else if ((free_asmpt_var.size() == 1) && (asmpt->get_op().is_null())) {
      write_single_term(outfile_asmpt, asmpt);
    } else {
      write_term(outfile_asmpt, asmpt);
    }

    const auto & asmpt_interp = state.get_assumption_interpretations().at(i);
    write_string(outfile_asmpt, asmpt_interp);
  }
}

StateAsmpt StateRW::StateRead(const std::string & infile_sv, const std::string & infile_asmpt)
{
  // 1. read sv
  std::ifstream f(infile_sv.c_str());
  std::string temp_file = "temp_sv.log";
  std::string linedata = "";
  smt::Term input_term;
  smt::TermVec var_vec = {};
  smt::TermVec value_vec = {};
  smt::UnorderedTermMap state_sv = {};
  bool even_sv = false;
  while (f) {
    getline(f, linedata);
    if (f.fail()) {
      break;
    }
    std::ofstream temp;
    temp.open(temp_file.c_str(), ios::out);
    temp << linedata << endl;
    #error "fixme" // why use two ways to read/write?
    if (std::regex_match(linedata, regex("#b(\\d+)"))) {
      input_term = str2bvnum(linedata);
    } else {
      WasimSmtLib2Parser pi(temp_file, solver_);
      input_term = pi.return_defs();
    }

    // cout << input_term->to_string() << endl;

    /// check the parity of cnt
    /// cnt is odd -> this input_term is sv (LHS)
    /// cnt is even -> this input_term is value (RHS)
    if (!even_sv) {
      var_vec.push_back(input_term);
    } else {
      value_vec.push_back(input_term);
    }
    even_sv = !even_sv;
  }
  f.close();
  int rm_sv;
  rm_sv = remove(temp_file.c_str());

  // finish read sv, then combine the two vectors to a map
  // cout << var_vec.size() << value_vec.size() << endl;
  assert(var_vec.size() == value_vec.size());
  for (int i = 0; i < var_vec.size(); i++) {
    auto key = var_vec.at(i);
    auto value = value_vec.at(i);
    state_sv[key] = value;
  }

  // 2. read asmpt
  std::ifstream f_asmpt(infile_asmpt.c_str());
  std::string temp_file_asmpt = "temp_asmpt.log";
  std::string linedata_asmpt = "";
  smt::Term input_asmpt;
  smt::TermVec state_asmpt_vec;
  std::vector<std::string> state_asmpt_interp_vec;
  bool even_asmpt = false;
  while (f_asmpt) {
    getline(f_asmpt, linedata_asmpt);
    if (f_asmpt.fail()) {
      break;
    }
    std::ofstream temp_asmpt;
    temp_asmpt.open(temp_file_asmpt.c_str(), ios::out);
    temp_asmpt << linedata_asmpt << endl;
    if (!even_asmpt) {
      if (std::regex_match(linedata_asmpt, regex("#b(\\d+)"))) {
        input_asmpt = str2bvnum(linedata_asmpt);
      } else {
        WasimSmtLib2Parser pi(temp_file_asmpt, solver_);
        input_asmpt = pi.return_defs();
      }
      state_asmpt_vec.push_back(input_asmpt);
    } else {
      state_asmpt_interp_vec.push_back(linedata_asmpt);
    }
    even_asmpt = !even_asmpt;
  }

  f_asmpt.close();
  // cout << state_asmpt_vec.size() << " ->- " << state_asmpt_interp_vec.size()
  // << endl;
  // 3. build a new state
  int rm_asmpt;
  rm_asmpt = remove(temp_file_asmpt.c_str());
  StateAsmpt state_ret(state_sv, state_asmpt_vec, state_asmpt_interp_vec);
  return state_ret;
}

void StateRW::StateWriteTree(const std::vector<std::vector<StateAsmpt>> & branch_list,
                             const std::string & out_dir)
{
  for (int i = 0; i < branch_list.size(); i++) {
    auto state_list = branch_list.at(i);
    for (int j = 0; j < state_list.size(); j++) {
      auto state = state_list.at(j);
      std::string outfile_sv =
          out_dir + "state_sv_" + to_string(i) + "_" + to_string(j);
      std::string outfile_asmpt =
          out_dir + "state_asmpt_" + to_string(i) + "_" + to_string(j);
      StateWrite(state, outfile_sv, outfile_asmpt);
      // cout << "i = " << to_string(i) << ", j = " << to_string(j) << endl;
      // cout << outfile_sv << endl;
    }
  }
}

std::vector<std::vector<StateAsmpt>> StateRW::StateReadTree(const std::string & in_dir,
                                                            int num_i,
                                                            int num_j)
{
  cout << "Reading files. Please wait for seconds." << endl;
  std::vector<std::vector<StateAsmpt>> branch_list = {};
  for (int i = 0; i < num_i; i++) {
    std::vector<StateAsmpt> state_list = {};
    for (int j = 0; j < num_j; j++) {
      std::string infile_sv =
          in_dir + "state_sv_" + to_string(i) + "_" + to_string(j);
      std::string infile_asmpt =
          in_dir + "state_asmpt_" + to_string(i) + "_" + to_string(j);
      // std::string infile_asmpt = "";
      auto state = StateRead(infile_sv, infile_asmpt);
      state_list.push_back(state);

      // cout << "i = " << i << ", j = " << j << endl;
      // cout << infile_sv << endl;
    }
    branch_list.push_back(state_list);
  }
  cout << "Reading complete!" << endl;

  return branch_list;
}


} // end of namespace wasim
