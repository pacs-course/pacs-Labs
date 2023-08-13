#ifndef HAVE_ADAPTIVE_QUADRATURE_HPP
#define HAVE_ADAPTIVE_QUADRATURE_HPP

#include <functional>

extern "C" {
double integrate(const std::function<double(double)> &, double, double);
}

#endif
