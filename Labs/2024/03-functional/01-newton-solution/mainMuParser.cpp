#include "NewtonSolver.hpp"
#include "muparser_fun.hpp"

int main(int argc, char **argv)
{
  const unsigned int max_it = 50;
  const double tol_res = 1e-8;
  const double tol_x = 1e-8;

  MuparserFun fun("x*x*x + 5*x + 3");
  MuparserFun dfun("3*x*x + 5");

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
