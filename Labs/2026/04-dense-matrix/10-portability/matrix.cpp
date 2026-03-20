#include "matrix.hpp"

#include <algorithm>
#include <cassert>
#include <vector>

#ifdef USE_THRUST
#  include <thrust/copy.h>
#  include <thrust/device_ptr.h>
#  include <thrust/device_vector.h>
#  include <thrust/execution_policy.h>
#  include <thrust/for_each.h>
#  include <thrust/iterator/counting_iterator.h>

#  define host_vector std::vector
#  define device_vector thrust::device_vector
#  define vector_copy thrust::copy
#  define raw_pointer_cast(ptr) thrust::raw_pointer_cast(ptr)
#  define HOST_DEVICE __host__ __device__
#  define PARALLEL_FOR(total, functor)                                            \
    thrust::for_each(thrust::device,                                               \
                     thrust::make_counting_iterator(0),                            \
                     thrust::make_counting_iterator(total),                        \
                     functor)
#else
#  include <execution>
#  include <ranges>

#  define host_vector std::vector
#  define device_vector std::vector
#  define vector_copy std::copy
#  define raw_pointer_cast(ptr) (ptr)
#  define HOST_DEVICE
#  define PARALLEL_FOR(total, functor)                                            \
    do                                                                             \
      {                                                                            \
        const auto indices = std::views::iota(0, total);                           \
        std::for_each(std::execution::par, indices.begin(), indices.end(), functor); \
      }                                                                            \
    while (false)
#endif

namespace
{
struct transpose_functor
{
  const double *in;
  double *out;
  int in_rows;
  int out_rows;
  int out_cols;

  HOST_DEVICE void
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

  HOST_DEVICE void
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

  host_vector<double> h_in(data.begin(), data.end());
  device_vector<double> d_in(h_in.size());
  device_vector<double> d_out(retval.get_rows() * retval.get_cols());

  vector_copy(h_in.begin(), h_in.end(), d_in.begin());

  const int total = static_cast<int>(retval.get_rows() * retval.get_cols());
  transpose_functor f{raw_pointer_cast(d_in.data()),
                      raw_pointer_cast(d_out.data()),
                      static_cast<int>(get_rows()),
                      static_cast<int>(retval.get_rows()),
                      static_cast<int>(retval.get_cols())};

  PARALLEL_FOR(total, f);

  vector_copy(d_out.begin(), d_out.end(), retval.get_data());
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

  host_vector<double> h_A(A.get_data(), A.get_data() + M * K);
  host_vector<double> h_B(B.get_data(), B.get_data() + K * N);
  device_vector<double> d_A(h_A.size());
  device_vector<double> d_B(h_B.size());
  device_vector<double> d_C(M * N, 0.0);

  vector_copy(h_A.begin(), h_A.end(), d_A.begin());
  vector_copy(h_B.begin(), h_B.end(), d_B.begin());

  const int total = M * N;
  matmul_functor f{
    raw_pointer_cast(d_A.data()), raw_pointer_cast(d_B.data()), raw_pointer_cast(d_C.data()), M, N, K};

  PARALLEL_FOR(total, f);

  vector_copy(d_C.begin(), d_C.end(), retval.get_data());
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
