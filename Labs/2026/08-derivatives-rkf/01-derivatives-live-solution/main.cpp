/*
NthDerivative<3> is recurs. built as first der. of NthDerivative<2>
 - template recursion
 - specialisation (for stopping case)
*/

#include <cmath>
#include <iostream>
#include "Derivatives.hpp"
#include <functional>

int main ()
{
  
  auto f = [] (const double &x) {return std::exp(x);};
  double h = 0.01;
/*
  NthDerivative <3, decltype (f)> d3 {f, h};

  std::cout << d3 (1.0) << std::endl; // 2.7128
*/
  
  std::cout << NthDerivative <3, decltype (f)> {f, h} (1.) << std::endl; // 2.7128
  
  
  return 0;
}