#include "NewtonSolver.hpp"
#include "GetPot"
#include "muparser_fun.hpp"
#include <string>


int main(int argc, char **argv)
{

  GetPot            command_line(argc, argv);
  const std::string filename = command_line.follow("data", 2, "-f", "--file");

  GetPot            datafile(filename.c_str());
  const std::string section = "NewtonSolver/";

  const unsigned int max_it = datafile((section + "max_it").data(), 0.0);
  const double tol_res = datafile((section + "tol_res").data(), 0.0);
  const double tol_x = datafile((section + "tol_x").data(), 0.0);

  std::string funString = datafile((section + "fun").data(), " ");
  std::string dfunString = datafile((section + "dfun").data(), " ");

  MuparserFun fun(funString);
  MuparserFun dfun(dfunString);


  // initialize solver
  NewtonSolver solver(fun, dfun, max_it, tol_res, tol_x);

  // run Newton solver with initial condition 0.0 
  solver.solve(0.0);

  // output the results
  std::cout << "x =    " << solver.get_result() << std::endl;
  std::cout << "r =    " << solver.get_residual() << std::endl;
  std::cout << "iter = " << solver.get_iter() << std::endl;

  return 0;
}
