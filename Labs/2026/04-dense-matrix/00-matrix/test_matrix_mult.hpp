#ifndef HAVE_TEST_MATRIX_MULT_H
#define HAVE_TEST_MATRIX_MULT_H

#ifndef msize
#  define msize 100
#endif

#include <chrono>
#include <iostream>

static std::chrono::steady_clock::time_point timer_start;

inline void
tic()
{
  timer_start = std::chrono::steady_clock::now();
}

inline double
toc_seconds()
{
  const auto timer_end = std::chrono::steady_clock::now();
  return std::chrono::duration<double>(timer_end - timer_start).count();
}

inline void
toc(const char *label)
{
  std::cout << label << toc_seconds() << " [s]" << std::endl;
}

inline double
dgemm_gflops(unsigned int m, unsigned int n, unsigned int k, double seconds)
{
  const double flops = 2.0 * static_cast<double>(m) * static_cast<double>(n) *
                       static_cast<double>(k);
  return flops / (seconds * 1.0e9);
}

inline void
print_dgemm_gflops(const char *label,
                   unsigned int m,
                   unsigned int n,
                   unsigned int k,
                   double seconds)
{
  std::cout << label << dgemm_gflops(m, n, k, seconds) << " [GFLOP/s]"
            << std::endl;
}

#endif
