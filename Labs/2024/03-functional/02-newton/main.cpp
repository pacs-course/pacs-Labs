#include "NewtonSolver.hpp"

// function f 
double fun(const double &x)
{
  return x*x*x + 5 * x + 3;
};

// derivative of f
double dfun(const double &x)
{
  return 3 *x*x + 5;
};

int main(int argc, char **argv)
{
  // initialize solver
  NewtonSolver solver(fun, dfun);

  // run Newton solver with initial condition 0.0 
  solver.solve(0.0);

  // output the results
  std::cout << "x =    " << solver.get_result() << std::endl;
  std::cout << "r =    " << solver.get_residual() << std::endl;
  std::cout << "iter = " << solver.get_iter() << std::endl;

  return 0;
}
