#include <chrono>
#include <cstdlib> // atoll

#include "matrix.hpp"

constexpr size_t default_matrix_size = 1000;

int main(int argc, char **argv) {
  // parse matrix size if given, otherwise use default value
  const size_t matrix_size =
      argc > 1 ? std::atoll(argv[1]) : default_matrix_size;
  std::cout << "matrix_size = " << matrix_size << std::endl;

  // fill the two matrix to multiply
  matrix A(matrix_size);
  matrix B(matrix_size);
  for (size_t i = 0; i < matrix_size; ++i) {
    A(i, i) = 10.0;
    A(i, matrix_size - 1) = 30.0;
    B(i, 0) = 1.0;
    B(i, i) = 3.0;
  }

  using namespace std::chrono;
  // test naive matrix multiplication
  {
    const auto t0 = high_resolution_clock::now();
    matrix C = A * B;
    const auto t1 = high_resolution_clock::now();
    const auto dt = duration_cast<milliseconds>(t1 - t0).count();
    std::cout << "Multiply time: " << dt << " [ms] " << std::endl;
  }

  // test matrix multiplication with transpose
  {
    const auto t0 = high_resolution_clock::now();
    matrix C = A.mmult_transpose_trick(B);
    const auto t1 = high_resolution_clock::now();
    const auto dt = duration_cast<milliseconds>(t1 - t0).count();
    std::cout << "Multiply (transpose trick) time: " << dt << " [ms] "
              << std::endl;
  }

  // test matrix multiplication done by eigen
  {
    const auto t0 = high_resolution_clock::now();
    matrix C = A.mmult_eigen(B);
    const auto t1 = high_resolution_clock::now();
    const auto dt = duration_cast<milliseconds>(t1 - t0).count();
    std::cout << "Multiply (eigen) time: " << dt << " [ms] " << std::endl;
  }

  // test time to transpose matrix
  {
    const auto t0 = high_resolution_clock::now();
    matrix D = A.transpose();
    const auto t1 = high_resolution_clock::now();
    const auto dt = duration_cast<milliseconds>(t1 - t0).count();
    std::cout << "Transpose time: " << dt << " [ms] " << std::endl;
  }

  return 0;
}
