#include "matrix.hpp"

#include <algorithm>
#include <cassert>
#include <stdexcept>

#include <cuda_runtime.h>
#include <thrust/copy.h>
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

namespace
{
constexpr int tile_size = 16;

void
check_cuda(cudaError_t err, const char *what)
{
  if (err != cudaSuccess)
    throw std::runtime_error(std::string(what) + ": " +
                             cudaGetErrorString(err));
}

__global__ void
transpose_kernel(const double *in, double *out, int in_rows, int in_cols)
{
  __shared__ double tile[tile_size][tile_size + 1];

  const int x = blockIdx.x * tile_size + threadIdx.x;
  const int y = blockIdx.y * tile_size + threadIdx.y;

  if (x < in_cols && y < in_rows)
    tile[threadIdx.y][threadIdx.x] = in[y + x * in_rows];

  __syncthreads();

  const int out_row = blockIdx.x * tile_size + threadIdx.y;
  const int out_col = blockIdx.y * tile_size + threadIdx.x;

  if (out_row < in_cols && out_col < in_rows)
    out[out_row + out_col * in_cols] = tile[threadIdx.x][threadIdx.y];
}

__global__ void
matmul_kernel(const double *A, const double *B, double *C, int M, int N, int K)
{
  __shared__ double tile_a[tile_size][tile_size];
  __shared__ double tile_b[tile_size][tile_size];

  const int row = blockIdx.y * tile_size + threadIdx.y;
  const int col = blockIdx.x * tile_size + threadIdx.x;

  double sum = 0.0;

  for (int kk = 0; kk < K; kk += tile_size)
    {
      const int a_col = kk + threadIdx.x;
      const int b_row = kk + threadIdx.y;

      tile_a[threadIdx.y][threadIdx.x] =
        (row < M && a_col < K) ? A[row + a_col * M] : 0.0;
      tile_b[threadIdx.y][threadIdx.x] =
        (b_row < K && col < N) ? B[b_row + col * K] : 0.0;

      __syncthreads();

      for (int inner = 0; inner < tile_size; ++inner)
        sum += tile_a[threadIdx.y][inner] * tile_b[inner][threadIdx.x];

      __syncthreads();
    }

  if (row < M && col < N)
    C[row + col * M] = sum;
}
} // namespace

matrix
matrix::transpose() const
{
  matrix retval(get_cols(), get_rows());

  thrust::host_vector<double> h_in(data.begin(), data.end());
  thrust::device_vector<double> d_in = h_in;
  thrust::device_vector<double> d_out(retval.get_rows() * retval.get_cols());

  const dim3 block(tile_size, tile_size);
  const dim3 grid((static_cast<int>(get_cols()) + tile_size - 1) / tile_size,
                  (static_cast<int>(get_rows()) + tile_size - 1) / tile_size);

  transpose_kernel<<<grid, block>>>(thrust::raw_pointer_cast(d_in.data()),
                                    thrust::raw_pointer_cast(d_out.data()),
                                    static_cast<int>(get_rows()),
                                    static_cast<int>(get_cols()));
  check_cuda(cudaGetLastError(), "transpose_kernel launch");
  check_cuda(cudaDeviceSynchronize(), "transpose_kernel synchronize");

  thrust::copy(d_out.begin(), d_out.end(), retval.get_data());
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

  thrust::device_vector<double> d_A(A.get_data(), A.get_data() + M * K);
  thrust::device_vector<double> d_B(B.get_data(), B.get_data() + K * N);
  thrust::device_vector<double> d_C(M * N, 0.0);

  const dim3 block(tile_size, tile_size);
  const dim3 grid((N + tile_size - 1) / tile_size,
                  (M + tile_size - 1) / tile_size);

  matmul_kernel<<<grid, block>>>(thrust::raw_pointer_cast(d_A.data()),
                                 thrust::raw_pointer_cast(d_B.data()),
                                 thrust::raw_pointer_cast(d_C.data()),
                                 M,
                                 N,
                                 K);
  check_cuda(cudaGetLastError(), "matmul_kernel launch");
  check_cuda(cudaDeviceSynchronize(), "matmul_kernel synchronize");

  thrust::copy(d_C.begin(), d_C.end(), retval.get_data());
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
