#include <dlfcn.h>

#include <cmath>
#include <functional>
#include <iostream>
#include <numbers>

double integrand(double x) { return std::sin(x * x) * std::sin(x * x); }

int main(int argc, char **argv) {
  const double res = integrate(integrand, 0, std::numbers::pi_v<double>);

  std::cout << "res = " << res << std::endl;

  return 0;
}
