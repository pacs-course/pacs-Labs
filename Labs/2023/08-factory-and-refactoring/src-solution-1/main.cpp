#include <Eigen/Dense>
#include <cmath>
#include <fstream>
#include <iostream>

#include "RKF.hpp"

void test_exp() {
  const auto f = [](const double& t, const double& y) { return -10 * y; };
  const auto sol_exact = [](const double& t) { return std::exp(-10 * t); };

  RKFOptions<double> options("data_exp.pot");
  RKF<RKFScheme::RK23_t, double> solver(f, options);
  const auto solution = solver.solve();

  // Compute error.
  double max_error = 0;
  unsigned int count = 0;

  for (const auto val : solution.y)
    max_error =
        std::max(max_error, std::abs(val - sol_exact(solution.time[count++])));

  std::cout << std::boolalpha;
  std::cout << "*** Model problem ***" << std::endl
            << "  l_inf error: " << max_error << std::endl
            << "  Failed: " << solution.failed << std::endl
            << "  Error estimate: " << solution.error_estimate << std::endl;
  std::cout << std::endl;

  std::ofstream file("data/results_exp.out");
  file << solution;
}

void test_VdP() {
  const auto f = [](const double& t, const Eigen::VectorXd& y) {
    const double mu = 1;

    Eigen::VectorXd out(2);
    out(0) = y(1);
    out(1) = -y(0) + mu * (1 - y(0) * y(0)) * y(1);

    return out;
  };

  using Variable = Eigen::VectorXd;

  RKFOptions<Variable> options("data_VdP.pot");
  RKF<RKFScheme::RK45_t, Variable> solver(f, options);
  const auto solution = solver.solve();

  std::cout << std::boolalpha;
  std::cout << "*** Van der Pol oscillator ***" << std::endl
            << "  Failed: " << solution.failed << std::endl
            << "  Error estimate: " << solution.error_estimate << std::endl;
  std::cout << std::endl;

  std::ofstream file("data/results_VdP.out");
  file << solution;
}

int main(int argc, char** argv) {
  // Model problem.
  test_exp();

  // Van der Pol oscillator problem with mu = 1.
  test_VdP();

  return 0;
}
