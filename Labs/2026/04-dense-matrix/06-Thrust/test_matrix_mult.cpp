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

  tic();
  matrix C = A * B;
  const double multiply_sec = toc_seconds();
  std::cout << "multiply_time = " << multiply_sec << " [s]" << std::endl;
  print_dgemm_gflops("multiply_perf = ",
                     A.get_rows(),
                     B.get_cols(),
                     A.get_cols(),
                     multiply_sec);

  tic();
  matrix D = A.transpose();
  toc("transpose_time = ");

  return 0;
}
