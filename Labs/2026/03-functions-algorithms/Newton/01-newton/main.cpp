#include "newton.hpp"
#include <iostream>

/*constructor takes
const std::function<double(const double &)> &fun_,
                const std::function<double(const double &)> &dfun_,
                */

/* //function pointers
double fun (const double & x) { return x*x*x + 5*x +3;}
double dfun (const double & x) { return 3*x*x+5;}
*/

struct Fun
{
  double operator () (double x) const  {return x*x*x + 5*x +3;}
};

struct DFun
{
  double operator () (double x) const  {return 3*x*x+5;}
};


int
main(int argc, char **argv)
{
/* //lambdas
  auto fun = [](const double & x) -> double { return x*x*x + 5*x +3;};
  auto dfun = [](const double & x) { return 3*x*x+5;};
*/

  Fun fun;
  DFun dfun;
  
  NewtonSolver solver (fun, dfun);
  
  solver.solve (0.);
  
  std::cout << solver.get_result () << std::endl;
  std::cout << solver.get_residual () << std::endl;
  std::cout << solver.get_iter () << std::endl;

  return 0;
}
