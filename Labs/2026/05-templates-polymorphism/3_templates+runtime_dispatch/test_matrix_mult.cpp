#include "test_matrix_mult.hpp"

#include "matrix.hpp"

#include <exception>
#include <iostream>

namespace runtime_portability_demo
{
enum class backend : int;

backend parse_backend(std::string_view name);
matrix  multiply(const matrix &A, const matrix &B, backend selected_backend);
matrix  transpose(const matrix &input, backend selected_backend);
} // namespace runtime_portability_demo

int
main(int argc, char **argv)
{
  if (argc != 2)
    {
      std::cerr << "usage: " << argv[0] << " <stl|thrust>" << std::endl;
      return 1;
    }

  const runtime_portability_demo::backend selected_backend =
    runtime_portability_demo::parse_backend(argv[1]);

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

  try
    {
      tic();
      matrix C = runtime_portability_demo::multiply(A, B, selected_backend);
      const double multiply_sec = toc_seconds();
      std::cout << "multiply_time = " << multiply_sec << " [s]" << std::endl;
      print_dgemm_gflops("multiply_perf = ",
                         A.get_rows(),
                         B.get_cols(),
                         A.get_cols(),
                         multiply_sec);

      tic();
      matrix D = runtime_portability_demo::transpose(A, selected_backend);
      toc("transpose_time = ");
    }
  catch (const std::exception &ex)
    {
      std::cerr << ex.what() << std::endl;
      return 1;
    }

  return 0;
}
