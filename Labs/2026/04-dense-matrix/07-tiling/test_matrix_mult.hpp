#ifndef HAVE_TEST_MATRIX_MULT_H
#define HAVE_TEST_MATRIX_MULT_H

#ifndef msize
#  define msize 1000
#endif

#ifndef warmup_iters
#  define warmup_iters 2
#endif

#ifndef benchmark_iters
#  define benchmark_iters 5
#endif

#include <cstdlib>
#include <chrono>
#include <iostream>

#include <cuda_runtime.h>

static std::chrono::steady_clock::time_point timer_start;

inline void
check_cuda(cudaError_t err, const char *what)
{
  if (err != cudaSuccess)
    {
      std::cerr << what << ": " << cudaGetErrorString(err) << std::endl;
      std::abort();
    }
}

inline void
tic()
{
  timer_start = std::chrono::steady_clock::now();
}

inline double
toc_seconds()
{
  check_cuda(cudaDeviceSynchronize(), "cudaDeviceSynchronize");
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

template <class Fn>
double
benchmark_average_seconds(Fn &&fn,
                          unsigned int warmups = warmup_iters,
                          unsigned int iterations = benchmark_iters)
{
  for (unsigned int i = 0; i < warmups; ++i)
    fn();

  tic();
  for (unsigned int i = 0; i < iterations; ++i)
    fn();
  return toc_seconds() / static_cast<double>(iterations);
}

#endif
