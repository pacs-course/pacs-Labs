#include <cmath>
#include <exception>
#include <fstream>
#include <iostream>

#include "RKF.hpp"

struct Array : public std::array<double, 3> {
  double norm() const {
    return std::sqrt((*this)[0] * (*this)[0] + (*this)[1] * (*this)[1] +
                     (*this)[2] * (*this)[2]);
  }
  Array& operator+=(const Array& rhs) {
    (*this)[0] += rhs[0];
    (*this)[1] += rhs[1];
    (*this)[2] += rhs[2];
    return *this;
  }
  friend Array operator-(Array lhs, const Array& rhs) {
    return {{lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2]}};
  }
  friend Array operator*(double lhs, const Array& rhs) {
    return {{lhs * rhs[0], lhs * rhs[1], lhs * rhs[2]}};
  }
};

int main(int argc, char** argv) {
  const auto f = [](const double& t, const Array& y) {
    const double rho = 28;
    const double sigma = 10;
    const double beta = 8.0 / 3;
    return Array{{sigma * (y[1] - y[0]), y[0] * (rho - y[2]) - y[1],
                  y[0] * y[1] - beta * y[2]}};
  };

  RKFOptions<Array> options("data_Lorenz.pot");
  RKF<Array> solver(f, options);
  const auto solution = solver.solve();

  std::cout << std::boolalpha;
  std::cout << "*** Lorenz system ***" << std::endl
            << "  Failed: " << solution.failed << std::endl
            << "  Error estimate: " << solution.error_estimate << std::endl;
  std::cout << std::endl;

  std::ofstream file("data/results_Lorenz.out");
  file << solution;

  return 0;
}
