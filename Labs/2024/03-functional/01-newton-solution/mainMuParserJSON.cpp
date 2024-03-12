#include "NewtonSolver.hpp"
#include "muparser_fun.hpp"
#include "json.hpp"
#include <fstream>

using json = nlohmann::json;
#include <string>


int main(int argc, char **argv)
{
  std::ifstream f("data.json");
  json data = json::parse(f);

  std::string funString = data.value("fun","");
  std::string dfunString = data.value("dfun","");
  const unsigned int max_it =  data.value("max_it",100);
  const double tol_res = data.value("tol_res", 1e-7);
  const double tol_x = data.value("tol_x", 1e-7);

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
