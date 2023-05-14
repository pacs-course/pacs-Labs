#ifndef HAVE_MIDPOINT_HPP
#define HAVE_MIDPOINT_HPP

#include <functional>

extern "C" {
double integrate(const std::function<double(double)> &, double, double);
}

#endif
