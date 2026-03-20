#include "matrix.hpp"

#include <algorithm>
#include <cassert>

#include <thrust/copy.h>
#include <thrust/device_ptr.h>
#include <thrust/device_vector.h>
#include <thrust/execution_policy.h>
#include <thrust/for_each.h>
#include <thrust/iterator/counting_iterator.h>

namespace
{
struct transpose_functor
{
  const double *in;
  double *out;
  int in_rows;
  int out_rows;
  int out_cols;

  __host__ __device__ void
  operator()(int idx) const
  {
    const int i = idx % out_rows;
    const int j = idx / out_rows;
    out[i + j * out_rows] = in[j + i * in_rows];
  }
};

struct matmul_functor
{
  const double *A;
  const double *B;
  double *C;
  int M;
  int N;
  int K;

  __host__ __device__ void
  operator()(int idx) const
  {
    const int row = idx % M;
    const int col = idx / M;

    double sum = 0.0;
    for (int kk = 0; kk < K; ++kk)
      sum += A[row + kk * M] * B[kk + col * K];

    C[row + col * M] = sum;
  }
};
} // namespace

matrix
matrix::transpose() const
{
  matrix retval(get_cols(), get_rows());

  thrust::device_vector<double> d_in(data.begin(), data.end());
  thrust::device_vector<double> d_out(retval.get_rows() * retval.get_cols());

  const int total = static_cast<int>(retval.get_rows() * retval.get_cols());
  transpose_functor f{thrust::raw_pointer_cast(d_in.data()),
                      thrust::raw_pointer_cast(d_out.data()),
                      static_cast<int>(get_rows()),
                      static_cast<int>(retval.get_rows()),
                      static_cast<int>(retval.get_cols())};

  thrust::for_each(thrust::device,
                   thrust::make_counting_iterator(0),
                   thrust::make_counting_iterator(total),
                   f);

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

  const int total = M * N;
  matmul_functor f{thrust::raw_pointer_cast(d_A.data()),
                   thrust::raw_pointer_cast(d_B.data()),
                   thrust::raw_pointer_cast(d_C.data()),
                   M,
                   N,
                   K};

  thrust::for_each(thrust::device,
                   thrust::make_counting_iterator(0),
                   thrust::make_counting_iterator(total),
                   f);

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
