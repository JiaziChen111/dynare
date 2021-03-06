/*
 * Copyright (C) 2003-2016 Dynare Team
 *
 * This file is part of Dynare.
 *
 * Dynare is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Dynare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Dynare.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _PARSING_DRIVER_HH
#define _PARSING_DRIVER_HH

#ifdef _MACRO_DRIVER_HH
# error Impossible to include both ParsingDriver.hh and MacroDriver.hh
#endif

#include <string>
#include <vector>
#include <istream>
#include <stack>

#include "ModFile.hh"
#include "SymbolList.hh"

class ParsingDriver;
#include "ExprNode.hh"
#include "DynareBison.hh"

#include "ComputingTasks.hh"
#include "Shocks.hh"
#include "SigmaeInitialization.hh"
#include "NumericalInitialization.hh"
#include "DynamicModel.hh"

using namespace std;

// Declare DynareFlexLexer class
#ifndef __FLEX_LEXER_H
# define yyFlexLexer DynareFlexLexer
# include <FlexLexer.h>
# undef yyFlexLexer
#endif

//! The lexer class
/*! Actually it was necessary to subclass the DynareFlexLexer class generated by Flex,
  since the prototype for DynareFlexLexer::yylex() was not convenient.
*/
class DynareFlex : public DynareFlexLexer
{
public:
  DynareFlex(istream *in = 0, ostream *out = 0);

  //! The main lexing function
  Dynare::parser::token_type lex(Dynare::parser::semantic_type *yylval,
                                 Dynare::parser::location_type *yylloc,
                                 ParsingDriver &driver);

  //! The filename being parsed
  /*! The bison parser locations (begin and end) contain a pointer to that string */
  string filename;

  //! Increment the location counter given a token
  void location_increment(Dynare::parser::location_type *yylloc, const char *yytext);

  //! Count parens in dates statement
  int dates_parens_nb;
};

//! Drives the scanning and parsing of the .mod file, and constructs its abstract representation
/*! It is built along the guidelines given in Bison 2.3 manual. */
class ParsingDriver
{
private:
  //! Checks that a given symbol exists, and stops with an error message if it doesn't
  void check_symbol_existence(const string &name);

  //! Checks that a given symbol exists and is a parameter, and stops with an error message if it isn't
  void check_symbol_is_parameter(string *name);

  //! Checks that a given symbol was assigned within a Statement
  void check_symbol_is_statement_variable(string *name);

  //! Checks that a given symbol exists and is a endogenous or exogenous, and stops with an error message if it isn't
  void check_symbol_is_endogenous_or_exogenous(string *name);

  //! Helper to add a symbol declaration
  void declare_symbol(const string *name, SymbolType type, const string *tex_name, const vector<pair<string *, string *> *> *partition_value);

  //! Creates option "optim_opt" in OptionsList if it doesn't exist, else add a comma, and adds the option name
  void optim_options_helper(const string &name);
  void sampling_options_helper(const string &name);

  //! Stores temporary symbol table
  SymbolList symbol_list;

  //! The data tree in which to add expressions currently parsed
  DataTree *data_tree;

  //! The model tree in which to add expressions currently parsed
  /*! It is only a dynamic cast of data_tree pointer, and is therefore null if data_tree is not a ModelTree instance */
  ModelTree *model_tree;

  //! The dynamic model tree in which to add expressions currently parsed
  /*! It is only a dynamic cast of data_tree pointer, and is therefore null if data_tree is not a DynamicModel instance */
  DynamicModel *dynamic_model;

  //! Sets data_tree and model_tree pointers
  void set_current_data_tree(DataTree *data_tree_arg);

  //! Stores options lists
  OptionsList options_list;
  //! Temporary storage for trend elements
  ObservationTrendsStatement::trend_elements_t trend_elements;
  //! Temporary storage for filename list of ModelComparison (contains weights)
  ModelComparisonStatement::filename_list_t filename_list;
  //! Temporary storage for list of EstimationParams (from estimated_params* statements)
  vector<EstimationParams> estim_params_list;
  //! Temporary storage for list of OsrParams (from osr_params_block statements)
  vector<OsrParams> osr_params_list;
  //! Temporary storage of variances from optim_weights
  OptimWeightsStatement::var_weights_t var_weights;
  //! Temporary storage of covariances from optim_weights
  OptimWeightsStatement::covar_weights_t covar_weights;
  //! Temporary storage for deterministic shocks
  ShocksStatement::det_shocks_t det_shocks;
  //! Temporary storage for periods of deterministic shocks
  vector<pair<int, int> > det_shocks_periods;
  //! Temporary storage for values of deterministic shocks
  vector<expr_t> det_shocks_values;
  //! Temporary storage for variances of shocks
  ShocksStatement::var_and_std_shocks_t var_shocks;
  //! Temporary storage for standard errors of shocks
  ShocksStatement::var_and_std_shocks_t std_shocks;
  //! Temporary storage for covariances of shocks
  ShocksStatement::covar_and_corr_shocks_t covar_shocks;
  //! Temporary storage for correlations of shocks
  ShocksStatement::covar_and_corr_shocks_t corr_shocks;
  //! Temporary storage for Sigma_e rows
  SigmaeStatement::row_t sigmae_row;
  //! Temporary storage for Sigma_e matrix
  SigmaeStatement::matrix_t sigmae_matrix;
  //! Temporary storage for initval/endval blocks
  InitOrEndValStatement::init_values_t init_values;
  //! Temporary storage for histval blocks
  HistValStatement::hist_values_t hist_values;
  //! Temporary storage for homotopy_setup blocks
  HomotopyStatement::homotopy_values_t homotopy_values;
  //! Temporary storage for moment_calibration
  MomentCalibration::constraints_t moment_calibration_constraints;
  //! Temporary storage for irf_calibration
  IrfCalibration::constraints_t irf_calibration_constraints;
  //! Temporary storage for ramsey_constraints
  RamseyConstraintsStatement::constraints_t ramsey_constraints;
  //! Temporary storage for svar_identification blocks
  SvarIdentificationStatement::svar_identification_restrictions_t svar_ident_restrictions;
  //! Temporary storage for mapping the equation number to the restrictions within an svar_identification block
  map<int, vector<int> > svar_equation_restrictions;
  //! Temporary storage for restrictions in an equation within an svar_identification block
  vector<int> svar_restriction_symbols;
  //! Temporary storage for constants exculsion within an svar_identification 
  bool svar_constants_exclusion;
  //! Temporary storage for upper cholesky within an svar_identification block
  bool svar_upper_cholesky;
  //! Temporary storage for lower cholesky within an svar_identification block
  bool svar_lower_cholesky;
  //! Temporary storage for equation number for a restriction within an svar_identification block
  int svar_equation_nbr;
  //! Temporary storage for left/right handside of a restriction equation within an svar_identificaton block
  bool svar_left_handside;
  //! Temporary storage for current restriction number in svar_identification block
  map<int,int> svar_Qi_restriction_nbr;
  map<int,int> svar_Ri_restriction_nbr;
  //! Temporary storage for restriction type
  enum SvarRestrictionType
    {
      NOT_SET,
      Qi_TYPE,
      Ri_TYPE
    };
  SvarRestrictionType svar_restriction_type;

  //! Temporary storage for argument list of external function
  stack<vector<expr_t> >  stack_external_function_args;
  //! Temporary storage for parameters in joint prior statement
  vector<string> joint_parameters;
  //! Temporary storage for the symb_id associated with the "name" symbol of the current external_function statement
  int current_external_function_id;
  //! Temporary storage for option list provided to external_function()
  ExternalFunctionsTable::external_function_options current_external_function_options;
  //! Temporary storage for declaring trend variables
  vector<int> declared_trend_vars;
  //! Temporary storage for declaring nonstationary variables
  vector<int> declared_nonstationary_vars;
  //! Temporary storage for a variance declared in the prior statement
  expr_t prior_variance;
  SubsamplesStatement::subsample_declaration_map_t subsample_declaration_map;
  //! Temporary storage for subsample statement: map<pair<var_name1, var_name2>>, subsample_declaration_map >
  typedef map<pair<string, string >, SubsamplesStatement::subsample_declaration_map_t > subsample_declarations_t;
  subsample_declarations_t subsample_declarations;
  //! Temporary storage for shock_groups
  vector<string> shock_group;
  vector<ShockGroupsStatement::Group> shock_groups;
  //! Temporary storage for ramsey policy. Workaround for issue #1355
  vector<string> ramsey_policy_list;
  //! reset the values for temporary storage
  void reset_current_external_function_options();
  //! Adds a model lagged variable to ModelTree and VariableTable
  expr_t add_model_variable(int symb_id, int lag);
  //! For parsing the graph_format option
  SymbolList graph_formats;
  //! Temporary storage for equation tags
  vector<pair<string, string> > eq_tags;

  //! The mod file representation constructed by this ParsingDriver
  ModFile *mod_file;

  WarningConsolidation &warnings;

  bool nostrict;

public:
  ParsingDriver(WarningConsolidation &warnings_arg, bool nostrict_arg) : warnings(warnings_arg), nostrict(nostrict_arg) { };

  //! Starts parsing, and constructs the MOD file representation
  /*! The returned pointer should be deleted after use */
  ModFile *parse(istream &in, bool debug);

  //! Reference to the lexer
  class DynareFlex *lexer;

  //! Copy of parsing location, maintained by YYLLOC_DEFAULT macro in DynareBison.yy
  Dynare::parser::location_type location;

  //! Estimation parameters
  EstimationParams estim_params;

  //! OSR parameters
  OsrParams osr_params;

  //! Temporary storage for the prior shape
  PriorDistributions prior_shape;

  //! Error handler with explicit location
  void error(const Dynare::parser::location_type &l, const string &m) __attribute__ ((noreturn));
  //! Error handler using saved location
  void error(const string &m) __attribute__ ((noreturn));
  //! Warning handler using saved location
  void warning(const string &m);

  //! Check if a given symbol exists in the parsing context, and is not a mod file local variable
  bool symbol_exists_and_is_not_modfile_local_or_external_function(const char *s);
  //! Sets mode of ModelTree class to use C output
  void use_dll();
  //! the modelis block decomposed
  void block();
  //! the model is stored in a binary file
  void byte_code();
  //! the static model is not computed
  void no_static();
  //! the differentiate_forward_vars option is enabled (for all vars)
  void differentiate_forward_vars_all();
  //! the differentiate_forward_vars option is enabled (for a subset of vars)
  void differentiate_forward_vars_some();
  //! cutoff option of model block
  void cutoff(string *value);
  //! mfs option of model block
  void mfs(string *value);
  //! Sets the FILENAME for the initial value in initval
  void initval_file(string *filename);
  //! Declares an endogenous variable
  void declare_endogenous(string *name, string *tex_name = NULL, vector<pair<string *, string *> *> *partition_value = NULL);
  //! Declares an exogenous variable
  void declare_exogenous(string *name, string *tex_name = NULL, vector<pair<string *, string *> *> *partition_value = NULL);
  //! Declares an exogenous deterministic variable
  void declare_exogenous_det(string *name, string *tex_name = NULL, vector<pair<string *, string *> *> *partition_value = NULL);
  //! Declares a parameter
  void declare_parameter(string *name, string *tex_name = NULL, vector<pair<string *, string *> *> *partition_value = NULL);
  //! Declares a statement local variable
  void declare_statement_local_variable(string *name);
  //! Completes a subsample statement
  void set_subsamples(string *name1, string *name2);
  //! Declares a subsample, assigning the value to name
  void set_subsample_name_equal_to_date_range(string *name, string *date1, string *date2);
  //! Checks that a subsample statement (and given name) were provided for the pair name1 & name2
  void check_subsample_declaration_exists(string *name1, string *subsample_name);
  void check_subsample_declaration_exists(string *name1, string *name2, string *subsample_name);
  //! Copies the set of subsamples from_name to_name
  void copy_subsamples(string *to_name1, string *to_name2, string *from_name1, string *from_name2);
  //! Declares declare_optimal_policy_discount_factor as a parameter and initializes it to exprnode
  void declare_optimal_policy_discount_factor_parameter(expr_t exprnode);
  //! Adds a predetermined_variable
  void add_predetermined_variable(string *name);
  //! Declares and initializes a local parameter
  void declare_and_init_model_local_variable(string *name, expr_t rhs);
  //! Changes type of a symbol
  void change_type(SymbolType new_type, vector<string *> *var_list);
  //! Adds a list of tags for the current equation
  void add_equation_tags(string *key, string *value);
  //! Adds a non-negative constant to DataTree
  expr_t add_non_negative_constant(string *constant);
  //! Adds a NaN constant to DataTree
  expr_t add_nan_constant();
  //! Adds an Inf constant to DataTree
  expr_t add_inf_constant();
  //! Adds a model variable to ModelTree and VariableTable
  expr_t add_model_variable(string *name);
  //! Adds an Expression's variable
  expr_t add_expression_variable(string *name);
  //! Adds a "periods" statement
  void periods(string *periods);
  //! Adds a "dsample" statement
  void dsample(string *arg1);
  //! Adds a "dsample" statement
  void dsample(string *arg1, string *arg2);
  //! Writes parameter intitialisation expression
  void init_param(string *name, expr_t rhs);
  //! Writes an initval block
  void init_val(string *name, expr_t rhs);
  //! Writes an histval block
  void hist_val(string *name, string *lag, expr_t rhs);
  //! Adds an entry in a homotopy_setup block
  /*! Second argument "val1" can be NULL if no initial value provided */
  void homotopy_val(string *name, expr_t val1, expr_t val2);
  //! Writes end of an initval block
  void end_initval(bool all_values_required);
  //! Writes end of an endval block
  void end_endval(bool all_values_required);
  //! Writes end of an histval block
  void end_histval(bool all_values_required);
  //! Writes end of an homotopy_setup block
  void end_homotopy();
  //! Begin a model block
  void begin_model();
  //! Writes a shocks statement
  void end_shocks(bool overwrite);
  //! Writes a mshocks statement
  void end_mshocks(bool overwrite);
  //! Adds a deterministic chock or a path element inside a conditional_forecast_paths block
  void add_det_shock(string *var, bool conditional_forecast);
  //! Adds a std error chock
  void add_stderr_shock(string *var, expr_t value);
  //! Adds a variance chock
  void add_var_shock(string *var, expr_t value);
  //! Adds a covariance chock
  void add_covar_shock(string *var1, string *var2, expr_t value);
  //! Adds a correlated chock
  void add_correl_shock(string *var1, string *var2, expr_t value);
  //! Adds a shock period range
  void add_period(string *p1, string *p2);
  //! Adds a shock period
  void add_period(string *p1);
  //! Adds a deterministic shock value
  void add_value(expr_t value);
  //! Adds a deterministic shock value
  /*! \param v a string containing a (possibly negative) numeric constant */
  void add_value(string *v);
  //! Writes a Sigma_e block
  void do_sigma_e();
  //! Ends row of Sigma_e block
  void end_of_row();
  //! Adds a constant element to current row of Sigma_e
  void add_to_row_const(string *v);
  //! Adds an expression element to current row of Sigma_e
  void add_to_row(expr_t v);
  //! Write a steady command
  void steady();
  //! Sets an option to a numerical value
  void option_num(const string &name_option, string *opt);
  //! Sets an option to a numerical value
  void option_num(const string &name_option, const string &opt);
  //! Sets an option to a numerical value
  void option_num(const string &name_option, string *opt1, string *opt2);
  //! Sets an option to a string value
  void option_str(const string &name_option, string *opt);
  //! Sets an option to a string value
  void option_str(const string &name_option, const string &opt);
  //! Sets an option to a date value
  void option_date(const string &name_option, string *opt);
  //! Sets an option to a date value
  void option_date(const string &name_option, const string &opt);
  //! Sets an option to a list of symbols (used in conjunction with add_in_symbol_list())
  void option_symbol_list(const string &name_option);
  //! Sets an option to a vector of integers
  void option_vec_int(const string &name_option, const vector<int> *opt);
  //! Indicates that the model is linear
  void linear();
  //! Adds a variable to temporary symbol list
  void add_in_symbol_list(string *tmp_var);
  //! Writes a rplot() command
  void rplot();
  //! Writes a stock_simul command
  void stoch_simul();
  //! Writes a simul command
  void simul();
  //! Writes check command
  void check();
  //! Writes model_info command
  void model_info();
  //! Writes estimated params command
  void estimated_params();
  //! Writes estimated params init command
  void estimated_params_init(bool use_calibration = false);
  //! Writes estimated params bound command
  void estimated_params_bounds();
  //! Adds a declaration for a user-defined external function
  void external_function();
  //! Sets an external_function option to a string value
  void external_function_option(const string &name_option, string *opt);
  //! Sets an external_function option to a string value
  void external_function_option(const string &name_option, const string &opt);
  //! Add a line in an estimated params block
  void add_estimated_params_element();
  //! Writes osr params bounds command
  void osr_params_bounds();
  //! Add a line in an osr params block
  void add_osr_params_element();
  //! Sets the frequency of the data
  void set_time(string *arg);
  //! Estimation Data
  void estimation_data();
  //! Sets the prior for a parameter
  void set_prior(string *arg1, string *arg2);
  //! Sets the joint prior for a set of parameters
  void set_joint_prior(vector<string *>*symbol_vec);
  //! Adds a parameters to the list of joint parameters
  void add_joint_parameter(string *name);
  //! Adds the variance option to its temporary holding place
  void set_prior_variance(expr_t variance=NULL);
  //! Copies the prior from_name to_name
  void copy_prior(string *to_declaration_type, string *to_name1, string *to_name2, string *to_subsample_name,
                  string *from_declaration_type, string *from_name1, string *from_name2, string *from_subsample_name);
  //! Sets the options for a parameter
  void set_options(string *arg1, string *arg2);
  //! Copies the options from_name to_name
  void copy_options(string *to_declaration_type, string *to_name1, string *to_name2, string *to_subsample_name,
                    string *from_declaration_type, string *from_name1, string *from_name2, string *from_subsample_name);
  //! Sets the prior for estimated std dev
  void set_std_prior(string *arg1, string *arg2);
  //! Sets the options for estimated std dev
  void set_std_options(string *arg1, string *arg2);
  //! Sets the prior for estimated correlation
  void set_corr_prior(string *arg1, string *arg2, string *arg3);
 //! Sets the options for estimated correlation
  void set_corr_options(string *arg1, string *arg2, string *arg3);
  //! Runs estimation process
  void run_estimation();
  //! Runs dynare_sensitivy()
  void dynare_sensitivity();
  //! Adds an optimization option (string value)
  void optim_options_string(string *name, string *value);
  //! Adds an optimization option (numeric value)
  void optim_options_num(string *name, string *value);
  //! Adds an sampling option (string value)
  void sampling_options_string(string *name, string *value);
  //! Adds an sampling option (numeric value)
  void sampling_options_num(string *name, string *value);
  //! Check that no observed variable has yet be defined
  void check_varobs();
  //! Add a new observed variable
  void add_varobs(string *name);
  //! Svar_Identification Statement
  void begin_svar_identification();
  void end_svar_identification();
  //! Svar_Identification Statement: match list of restrictions and equation number with lag
  void combine_lag_and_restriction(string *lag);
  //! Svar_Identification Statement: match list of restrictions with equation number
  void add_restriction_in_equation(string *equation);
  //! Svar_Identification Statement: add list of restriction symbol ids
  void add_in_svar_restriction_symbols(string *name);
  //! Svar_Identification Statement: add exclusions of constants
  void add_constants_exclusion();
  //! Svar_Identification Statement: add equation number for following restriction equations
  void add_restriction_equation_nbr(string *eq_nbr);
  //! Svar_Identification Statement: record presence of equal sign
  void add_restriction_equal();
  //! Svar_Idenditification Statement: add coefficient of a linear restriction (positive value) 
  void add_positive_restriction_element(expr_t value, string *variable, string *lag);
  //! Svar_Idenditification Statement: add unit coefficient of a linear restriction 
  void add_positive_restriction_element(string *variable, string *lag);
  //! Svar_Idenditification Statement: add coefficient of a linear restriction (negative value) 
  void add_negative_restriction_element(expr_t value, string *variable, string *lag);
  //! Svar_Idenditification Statement: add negative unit coefficient of a linear restriction
  void add_negative_restriction_element(string *variable, string *lag);
  //! Svar_Idenditification Statement: add restriction element
  void add_restriction_element(expr_t value, string *variable, string *lag);
  //! Svar_Identification Statement: check that restriction is homogenous
  void check_restriction_expression_constant(expr_t value);
  //! Svar_Identification Statement: restriction of form upper cholesky
  void add_upper_cholesky();
  //! Svar_Identification Statement: restriction of form lower cholesky
  void add_lower_cholesky();
  //! Svar_Global_Identification_Check Statement
  void add_svar_global_identification_check();
  //! Forecast Statement
  void forecast();
  void set_trends();
  void set_trend_element(string *arg1, expr_t arg2);
  void set_unit_root_vars();
  void optim_weights();
  void set_optim_weights(string *name, expr_t value);
  void set_optim_weights(string *name1, string *name2, expr_t value);
  void set_osr_params();
  void run_osr();
  void run_dynasave(string *filename);
  void run_dynatype(string *filename);
  void run_load_params_and_steady_state(string *filename);
  void run_save_params_and_steady_state(string *filename);
  void run_identification();
  void add_mc_filename(string *filename, string *prior = new string("1"));
  void run_model_comparison();
  //! Begin a planner_objective statement
  void begin_planner_objective();
  //! End a planner objective statement
  void end_planner_objective(expr_t expr);
  //! Ramsey model statement
  void ramsey_model();
  //! Ramsey constraints statement
  void add_ramsey_constraints_statement();
  //! Ramsey less constraint
  void ramsey_constraint_add_less(const string *name, const expr_t rhs);
  //! Ramsey greater constraint
  void ramsey_constraint_add_greater(const string *name, const expr_t rhs);
  //! Ramsey less or equal constraint
  void ramsey_constraint_add_less_equal(const string *name, const expr_t rhs);
  //! Ramsey greater or equal constraint
  void ramsey_constraint_add_greater_equal(const string *name, const expr_t rhs);
  //! Ramsey constraint helper function
  void add_ramsey_constraint(const string *name, BinaryOpcode op_code, const expr_t rhs);
  //! Ramsey policy statement
  void ramsey_policy();
  //! Discretionary policy statement
  void discretionary_policy();
  //! Adds a write_latex_dynamic_model statement
  void write_latex_dynamic_model();
  //! Adds a write_latex_static_model statement
  void write_latex_static_model();
  //! Adds a write_latex_original_model statement
  void write_latex_original_model();
  //! BVAR marginal density
  void bvar_density(string *maxnlags);
  //! BVAR forecast
  void bvar_forecast(string *nlags);
  //! SBVAR statement
  void sbvar();
  //! Markov Switching Statement: Estimation
  void ms_estimation();
  //! Markov Switching Statement: Simulation
  void ms_simulation();
  //! Markov Switching Statement: MDD
  void ms_compute_mdd();
  //! Markov Switching Statement: Probabilities
  void ms_compute_probabilities();
  //! Markov Switching Statement: IRF
  void ms_irf();
  //! Markov Switching Statement: Forecast
  void ms_forecast();
  //! Markov Switching Statement: Variance Decomposition
  void ms_variance_decomposition();
  //! Svar statement
  void svar();
  //! MarkovSwitching statement
  void markov_switching();
  //! Shock decomposition
  void shock_decomposition();
  //! Conditional forecast statement
  void conditional_forecast();
  //! Conditional forecast paths block
  void conditional_forecast_paths();
  //! Plot conditional forecast statement
  void plot_conditional_forecast(string *periods = NULL);
  //! Smoother on calibrated models
  void calib_smoother();
  //! Extended path
  void extended_path();
  //! Writes token "arg1=arg2" to model tree
  expr_t add_model_equal(expr_t arg1, expr_t arg2);
  //! Writes token "arg=0" to model tree
  expr_t add_model_equal_with_zero_rhs(expr_t arg);
  //! Writes token "arg1+arg2" to model tree
  expr_t add_plus(expr_t arg1, expr_t arg2);
  //! Writes token "arg1-arg2" to model tree
  expr_t add_minus(expr_t arg1,  expr_t arg2);
  //! Writes token "-arg1" to model tree
  expr_t add_uminus(expr_t arg1);
  //! Writes token "arg1*arg2" to model tree
  expr_t add_times(expr_t arg1,  expr_t arg2);
  //! Writes token "arg1/arg2" to model tree
  expr_t add_divide(expr_t arg1,  expr_t arg2);
  //! Writes token "arg1<arg2" to model tree
  expr_t add_less(expr_t arg1, expr_t arg2);
  //! Writes token "arg1>arg2" to model treeexpr_t
  expr_t add_greater(expr_t arg1, expr_t arg2);
  //! Writes token "arg1<=arg2" to model treeexpr_t
  expr_t add_less_equal(expr_t arg1, expr_t arg2);
  //! Writes token "arg1>=arg2" to model treeexpr_t
  expr_t add_greater_equal(expr_t arg1, expr_t arg2);
  //! Writes token "arg1==arg2" to model treeexpr_texpr_t
  expr_t add_equal_equal(expr_t arg1, expr_t arg2);
  //! Writes token "arg1!=arg2" to model treeexpr_texpr_t
  expr_t add_different(expr_t arg1, expr_t arg2);
  //! Writes token "arg1^arg2" to model tree
  expr_t add_power(expr_t arg1,  expr_t arg2);
  //! Writes token "E(arg1)(arg2)" to model tree
  expr_t add_expectation(string *arg1,  expr_t arg2);
  //! Writes token "exp(arg1)" to model tree
  expr_t add_exp(expr_t arg1);
  //! Writes token "log(arg1)" to model tree
  expr_t add_log(expr_t arg1);
  //! Writes token "log10(arg1)" to model tree
  expr_t add_log10(expr_t arg1);
  //! Writes token "cos(arg1)" to model tree
  expr_t add_cos(expr_t arg1);
  //! Writes token "sin(arg1)" to model tree
  expr_t add_sin(expr_t arg1);
  //! Writes token "tan(arg1)" to model tree
  expr_t add_tan(expr_t arg1);
  //! Writes token "acos(arg1)" to model tree
  expr_t add_acos(expr_t arg1);
  //! Writes token "asin(arg1)" to model tree
  expr_t add_asin(expr_t arg1);
  //! Writes token "atan(arg1)" to model tree
  expr_t add_atan(expr_t arg1);
  //! Writes token "cosh(arg1)" to model tree
  expr_t add_cosh(expr_t arg1);
  //! Writes token "sinh(arg1)" to model tree
  expr_t add_sinh(expr_t arg1);
  //! Writes token "tanh(arg1)" to model tree
  expr_t add_tanh(expr_t arg1);
  //! Writes token "acosh(arg1)" to model tree
  expr_t add_acosh(expr_t arg1);
  //! Writes token "asin(arg1)" to model tree
  expr_t add_asinh(expr_t arg1);
  //! Writes token "atanh(arg1)" to model tree
  expr_t add_atanh(expr_t arg1);
  //! Writes token "sqrt(arg1)" to model tree
  expr_t add_sqrt(expr_t arg1);
  //! Writes token "abs(arg1)" to model tree
  expr_t add_abs(expr_t arg1);
  //! Writes token "sign(arg1)" to model tree
  expr_t add_sign(expr_t arg1);
  //! Writes token "max(arg1,arg2)" to model tree
  expr_t add_max(expr_t arg1, expr_t arg2);
  //! Writes token "min(arg1,arg2)" to model tree
  expr_t add_min(expr_t arg1, expr_t arg2);
  //! Writes token "normcdf(arg1,arg2,arg3)" to model tree
  expr_t add_normcdf(expr_t arg1, expr_t arg2, expr_t arg3);
  //! Writes token "normcdf(arg,0,1)" to model tree
  expr_t add_normcdf(expr_t arg);
  //! Writes token "normpdf(arg1,arg2,arg3)" to model tree
  expr_t add_normpdf(expr_t arg1, expr_t arg2, expr_t arg3);
  //! Writes token "normpdf(arg,0,1)" to model tree
  expr_t add_normpdf(expr_t arg);
  //! Writes token "erf(arg)" to model tree
  expr_t add_erf(expr_t arg);
  //! Writes token "steadyState(arg1)" to model tree
  expr_t add_steady_state(expr_t arg1);
  //! Pushes empty vector onto stack when a symbol is encountered (mod_var or ext_fun)
  void push_external_function_arg_vector_onto_stack();
  //! Adds an external function argument
  void add_external_function_arg(expr_t arg);
  //! Adds an external function call node
  expr_t add_model_var_or_external_function(string *function_name, bool in_model_block);
  //! Adds a native statement
  void add_native(const string &s);
  //! Adds a native statement, first removing the set of characters passed in token (and everything after)
  void add_native_remove_charset(const char *s, const string &token);
  //! Adds a verbatim statement
  void add_verbatim(const string &s);
  //! Adds a verbatim statement, first removing the set of characters passed in token (and everything after)
  void add_verbatim_remove_charset(const char *s, const string &token);
  //! Resets data_tree and model_tree pointers to default (i.e. mod_file->expressions_tree)
  void reset_data_tree();
  //! Begin a steady_state_model block
  void begin_steady_state_model();
  //! Add an assignment equation in steady_state_model block
  void add_steady_state_model_equal(string *varname, expr_t expr);
  //! Add a multiple assignment equation in steady_state_model block
  void add_steady_state_model_equal_multiple(expr_t expr);
  //! Switches datatree
  void begin_trend();
  //! Declares a trend variable with its growth factor
  void declare_trend_var(bool log_trend, string *name, string *tex_name = NULL);
  //! Ends declaration of trend variable
  void end_trend_var(expr_t growth_factor);
  //! Declares a nonstationary variable with its deflator
  void declare_nonstationary_var(string *name, string *tex_name = NULL, vector<pair<string *, string *> *> *partition_value = NULL);
  //! Ends declaration of nonstationary variable
  void end_nonstationary_var(bool log_deflator, expr_t deflator);
  //! Add a graph format to the list of formats requested
  void add_graph_format(const string &name);
  //! Add the graph_format option to the OptionsList structure
  void process_graph_format_option();
  //! Model diagnostics
  void model_diagnostics();
  //! Processing the parallel_local_files option
  void add_parallel_local_file(string *filename);
  //! Add an item of a moment_calibration statement
  void add_moment_calibration_item(string *endo1, string *endo2, string *lags, vector<string *> *range);
  //! End a moment_calibration statement
  void end_moment_calibration();
  //! Add an item of an irf_calibration statement
  void add_irf_calibration_item(string *endo, string *periods, string *exo, vector<string *> *range);
  //! End a moment_calibration statement
  void end_irf_calibration();
  //! Add a shock to a group
  void add_shock_group_element(string *name);
  //! Add a set of shock groups
  void add_shock_group(string *name);
  //! End shock groups declaration
  void end_shock_groups(const string *name);
  //! Add an element to the ramsey policy list
  void add_to_ramsey_policy_list(string *name);
  void smoother2histval();
  void histval_file(string *filename);
  void perfect_foresight_setup();
  void perfect_foresight_solver();
  void prior_posterior_function(bool prior_func);
};

#endif // ! PARSING_DRIVER_HH
