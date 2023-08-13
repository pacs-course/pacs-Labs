#include "RKF.hpp"

#include <cmath>
#include <fstream>
#include <iostream>

#include <Eigen/Dense>

void test_exp() {
  const auto f = [](const double& t, const double& y) { return -10 * y; };
  const auto sol_exact = [](const double& t) { return std::exp(-10 * t); };

  const double t0 = 0;
  const double tf = 10;
  const double y0 = 1;
  const double h0 = 0.2;
  const double tolerance = 1e-4;
  const unsigned int n_max_steps = 1e4;

  RKF<RKFScheme::RK23_t, double> solver(f);

  const auto solution = solver.solve(t0, tf, y0, h0, tolerance, n_max_steps);

  // Compute error.
  double max_error = 0;
  unsigned int count = 0;

  for (const auto val : solution.y)
    max_error =
        std::max(max_error, std::abs(val - sol_exact(solution.time[count++])));

  std::cout << std::boolalpha;
  std::cout << "*** Model problem ***" << std::endl
            << "  l_inf error: " << max_error << std::endl
            << "  Tolerance: " << tolerance << std::endl
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

  const double t0 = 0;
  const double tf = 40;

  Eigen::VectorXd y0(2);
  y0[0] = 1;
  y0[1] = 1;

  const double h0 = 1.0;
  const double tolerance = 1e-4;
  const unsigned int n_max_steps = 5e3;

  RKF<RKFScheme::RK45_t, Eigen::VectorXd> solver(f);

  const auto solution = solver.solve(t0, tf, y0, h0, tolerance, n_max_steps);

  std::cout << std::boolalpha;
  std::cout << "*** Van der Pol oscillator ***" << std::endl
            << "  Tolerance: " << tolerance << std::endl
            << "  Failed: " << solution.failed << std::endl
            << "  Error estimate: " << solution.error_estimate << std::endl;
  std::cout << std::endl;

  std::ofstream file("data/results_VdP.out");
  file << solution;
}

/// Example of minimal Vector
// You should avoid inherinting from std container (when using polymorphism)
// due to the absence of a virtual destructor. Also remember the "Is-a" rule.
// Here we do it just for shortness
struct Array : public std::array<double, 2> {
  double norm() const {
    return std::sqrt((*this)[0] * (*this)[0] + (*this)[1] * (*this)[1]);
  }
  Array& operator+=(const Array& rhs) {
    (*this)[0] += rhs[0];
    (*this)[1] += rhs[1];
    return *this;
  }
  friend Array operator-(Array lhs, const Array& rhs) {
    return {{lhs[0] - rhs[0], lhs[1] - rhs[1]}};
  }
  friend Array operator*(double lhs, const Array& rhs) {
    return {{lhs * rhs[0], lhs * rhs[1]}};
  }
};

void test_custom_type() {
  // Play with the struct `Array` above by removing pieces and see what
  // happens. Try also to change VariabeType. In this case the flag
  // `-fconcepts-diagnostics-depth` is your friend
  using VariabeType = Array;
  const auto f = [](double t, const VariabeType& y) { return y; };
  RKF<RKFScheme::RK45_t, VariabeType> solver(f);
  const auto solution = solver.solve(0, 1, Array{}, 0, 0, 0);
  std::ofstream file("data/a.out");
  file << solution;
}

int main(int argc, char** argv) {
  // Test with custum type, try it yourself to change the custom type
  test_custom_type();

  // Model problem.
  test_exp();

  // Van der Pol oscillator problem with mu = 1.
  test_VdP();

  return 0;
}
