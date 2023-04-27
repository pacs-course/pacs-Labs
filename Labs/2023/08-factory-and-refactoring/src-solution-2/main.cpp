#include <Eigen/Dense>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>

#include "RKF.hpp"

struct Array : public std::array<double, 3> {
  double norm() const {
    return std::sqrt((*this)[0] * (*this)[0] + (*this)[1] * (*this)[1] +
                     (*this)[2] * (*this)[2]);
  }
  Array &operator+=(const Array &rhs) {
    (*this)[0] += rhs[0];
    (*this)[1] += rhs[1];
    (*this)[2] += rhs[2];
    return *this;
  }
  friend Array operator-(Array lhs, const Array &rhs) {
    return {{lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2]}};
  }
  friend Array operator*(double lhs, const Array &rhs) {
    return {{lhs * rhs[0], lhs * rhs[1], lhs * rhs[2]}};
  }
};

int main(int argc, char **argv) {
  const auto f_array = [](const double &t, const Array &y) {
    const double rho = 28;
    const double sigma = 10;
    const double beta = 8.0 / 3;
    return Array{{sigma * (y[1] - y[0]), y[0] * (rho - y[2]) - y[1],
                  y[0] * y[1] - beta * y[2]}};
  };

  const auto f_eigen = [](const double &t, const Eigen::VectorXd &y) {
    const double rho = 28;
    const double sigma = 10;
    const double beta = 8.0 / 3;

    Eigen::VectorXd out(3);
    out(0) = sigma * (y(1) - y(0));
    out(1) = y(0) * (rho - y(2)) - y(1);
    out(2) = y(0) * y(1) - beta * y(2);

    return out;
  };

  using namespace std::chrono;

  {
    const auto t1 = high_resolution_clock::now();

    RKFOptions<Eigen::VectorXd> options("data_Lorenz.pot");
    RKF<RKFScheme::RK45_t, Eigen::VectorXd> solver(f_eigen, options);
    const auto solution = solver.solve();

    const auto t2 = high_resolution_clock::now();
    std::cout << "Eigen solve took: "
              << duration_cast<milliseconds>(t2 - t1).count() << "ms\n";
  }

  const auto t1 = high_resolution_clock::now();

  RKFOptions<Array> options("data_Lorenz.pot");
  RKF<RKFScheme::RK45_t, Array> solver(f_array, options);
  const auto solution = solver.solve();

  const auto t2 = high_resolution_clock::now();
  std::cout << "Array solve took: "
            << duration_cast<milliseconds>(t2 - t1).count() << "ms\n";

  std::cout << std::boolalpha;
  std::cout << "*** Lorenz system ***" << std::endl
            << "  Failed: " << solution.failed << std::endl
            << "  Error estimate: " << solution.error_estimate << std::endl;
  std::cout << std::endl;

  std::ofstream file("data/results_Lorenz.out");
  file << solution;

  return 0;
}
