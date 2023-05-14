#include "adaptive_quadrature.hpp"

#include <cmath>
#include <limits>

inline double trapz(const std::function<double(double)> &f, double a,
                    double b) {
  return 0.5 * (b - a) * (f(a) + f(b));
}

double refine(const std::function<double(double)> &f, double a, double b,
              double old_integral, int depth) {
  constexpr double tol = 1e-12;
  constexpr int maxdepth = 40;

  const double c = 0.5 * (a + b);
  const double integral_lx = trapz(f, a, c);
  const double integral_rx = trapz(f, c, b);
  const double new_integral = integral_lx + integral_rx;

  if (depth >= maxdepth || std::abs(new_integral - old_integral) <= tol) {
    return new_integral;
  } else {
    return refine(f, a, c, integral_lx, depth + 1) +
           refine(f, c, b, integral_rx, depth + 1);
  }
}

double integrate(const std::function<double(double)> &f, double a, double b) {
  // always refine at least once
  return refine(f, a, b, std::numeric_limits<double>::quiet_NaN(), 0);
}
