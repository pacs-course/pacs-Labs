#include "test_matrix_mult.hpp"

#include "matrix.hpp"

int
main()
{
  matrix A(msize);
  matrix B(msize);
  for (unsigned int i = 0; i < msize; ++i)
    {
      A(i, i)         = 10.0;
      A(i, msize - 1) = 30.0;
      B(i, 0)         = 1.0;
      B(i, i)         = 3.0;
    }

  std::cout << "msize = " << msize << std::endl;
  std::cout << "warmup_iters = " << warmup_iters << std::endl;
  std::cout << "benchmark_iters = " << benchmark_iters << std::endl;

  // From this point on we use warmups and averaged timings, because faster kernels are more sensitive to measurement noise.
  double multiply_sec = benchmark_average_seconds([&]() {
    matrix C = A * B;
    (void)C;
  });
  std::cout << "multiply_time = " << multiply_sec << " [s]" << std::endl;
  print_dgemm_gflops("multiply_perf = ",
                     A.get_rows(),
                     B.get_cols(),
                     A.get_cols(),
                     multiply_sec);

  double transpose_sec = benchmark_average_seconds([&]() {
    matrix D = A.transpose();
    (void)D;
  });
  std::cout << "transpose_time = " << transpose_sec << " [s]" << std::endl;

  return 0;
}
