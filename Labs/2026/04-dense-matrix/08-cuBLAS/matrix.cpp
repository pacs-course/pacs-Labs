#include "matrix.hpp"

#include <algorithm>
#include <cassert>
#include <stdexcept>

#include <cublas_v2.h>
#include <cuda_runtime.h>

namespace
{
void
check_cuda(cudaError_t err, const char *what)
{
  if (err != cudaSuccess)
    throw std::runtime_error(std::string(what) + ": " +
                             cudaGetErrorString(err));
}

void
check_cublas(cublasStatus_t status, const char *what)
{
  if (status != CUBLAS_STATUS_SUCCESS)
    throw std::runtime_error(std::string(what) + " failed");
}
} // namespace

matrix
matrix::transpose() const
{
  matrix retval(get_cols(), get_rows());

  const int in_rows = static_cast<int>(get_rows());
  const int in_cols = static_cast<int>(get_cols());
  const int out_rows = static_cast<int>(retval.get_rows());
  const int out_cols = static_cast<int>(retval.get_cols());

  const size_t bytes_in = sizeof(double) * get_rows() * get_cols();
  const size_t bytes_out = sizeof(double) * retval.get_rows() * retval.get_cols();

  double *d_in = nullptr;
  double *d_out = nullptr;
  cublasHandle_t handle;

  check_cuda(cudaMalloc(&d_in, bytes_in), "cudaMalloc(d_in)");
  check_cuda(cudaMalloc(&d_out, bytes_out), "cudaMalloc(d_out)");
  check_cuda(cudaMemcpy(d_in, get_data(), bytes_in, cudaMemcpyHostToDevice),
             "cudaMemcpy H2D transpose");

  check_cublas(cublasCreate(&handle), "cublasCreate");

  const double alpha = 1.0;
  const double beta = 0.0;

  check_cublas(cublasDgeam(handle,
                           CUBLAS_OP_T,
                           CUBLAS_OP_N,
                           out_rows,
                           out_cols,
                           &alpha,
                           d_in,
                           in_rows,
                           &beta,
                           d_in,
                           out_rows,
                           d_out,
                           out_rows),
               "cublasDgeam");

  check_cuda(cudaMemcpy(retval.get_data(), d_out, bytes_out, cudaMemcpyDeviceToHost),
             "cudaMemcpy D2H transpose");

  cublasDestroy(handle);
  cudaFree(d_out);
  cudaFree(d_in);

  return retval;
}

matrix
operator*(const matrix &A, const matrix &B)
{
  const int M = static_cast<int>(A.get_rows());
  const int N = static_cast<int>(B.get_cols());
  const int K = static_cast<int>(A.get_cols());
  assert(K == static_cast<int>(B.get_rows()));

  matrix retval(M, N);

  const size_t bytes_a = sizeof(double) * A.get_rows() * A.get_cols();
  const size_t bytes_b = sizeof(double) * B.get_rows() * B.get_cols();
  const size_t bytes_c = sizeof(double) * retval.get_rows() * retval.get_cols();

  double *d_a = nullptr;
  double *d_b = nullptr;
  double *d_c = nullptr;
  cublasHandle_t handle;

  check_cuda(cudaMalloc(&d_a, bytes_a), "cudaMalloc(d_a)");
  check_cuda(cudaMalloc(&d_b, bytes_b), "cudaMalloc(d_b)");
  check_cuda(cudaMalloc(&d_c, bytes_c), "cudaMalloc(d_c)");

  check_cuda(cudaMemcpy(d_a, A.get_data(), bytes_a, cudaMemcpyHostToDevice),
             "cudaMemcpy H2D A");
  check_cuda(cudaMemcpy(d_b, B.get_data(), bytes_b, cudaMemcpyHostToDevice),
             "cudaMemcpy H2D B");

  check_cublas(cublasCreate(&handle), "cublasCreate");

  const double alpha = 1.0;
  const double beta = 0.0;

  check_cublas(cublasDgemm(handle,
                           CUBLAS_OP_N,
                           CUBLAS_OP_N,
                           M,
                           N,
                           K,
                           &alpha,
                           d_a,
                           M,
                           d_b,
                           K,
                           &beta,
                           d_c,
                           M),
               "cublasDgemm");

  check_cuda(cudaMemcpy(retval.get_data(), d_c, bytes_c, cudaMemcpyDeviceToHost),
             "cudaMemcpy D2H C");

  cublasDestroy(handle);
  cudaFree(d_c);
  cudaFree(d_b);
  cudaFree(d_a);

  return retval;
}

void
matrix::solve(matrix &rhs)
{
  unsigned int ii, jj, kk;
  double       f;

  // Factorize
  if (!factorized)
    {
      factorize();
      std::cout << "factorize !" << std::endl;
    }

  double *b = rhs.get_data();
  // Do Forward Substitution
  std::cout << "fwdsub !" << std::endl;
  for (ii = 0; ii < get_rows(); ++ii)
    {
      f = b[p[ii]];
      for (kk = 0; kk < ii; ++kk)
        f -= value(p[ii], kk) * b[p[kk]];
      b[p[ii]] = f;
    }

  // Do Backward Substitution
  std::cout << "bwdsub !" << std::endl;
  for (jj = 1; jj <= get_rows(); ++jj)
    {
      ii = get_rows() - jj;
      f  = b[p[ii]];
      for (kk = ii + 1; kk < get_cols(); ++kk)
        f -= value(p[ii], kk) * b[p[kk]];
      b[p[ii]] = f / value(p[ii], ii);
    }
}

void
matrix::factorize()
{
  p.resize(rows, 0);
  for (unsigned int ii = 0; ii < rows; ++ii)
    p[ii] = ii;

  int    m = this->get_rows();
  int    n = this->get_cols();
  int    ii, jj, kk;
  double pivot = 0., maxpivot = 0.;
  int    imaxpivot = 0;

  assert(m == n);
  for (ii = 0; ii < m - 1; ++ii)
    {
      maxpivot  = value(p[ii], ii);
      imaxpivot = ii;
      for (kk = ii + 1; kk < m; ++kk)
        if (value(p[kk], ii) > maxpivot)
          {
            maxpivot  = value(p[kk], ii);
            imaxpivot = kk;
          }

      if (imaxpivot != ii)
        std::swap(p[ii], p[imaxpivot]);

      pivot = value(p[ii], ii);
      for (jj = ii + 1; jj < m; ++jj)
        {
          value(p[jj], ii) = value(p[jj], ii) / pivot;

          for (kk = ii + 1; kk < n; ++kk)
            value(p[jj], kk) += -value(p[ii], kk) * value(p[jj], ii);
        }
    }
  factorized = true;
}
