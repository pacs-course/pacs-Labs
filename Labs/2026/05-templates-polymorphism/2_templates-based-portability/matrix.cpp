#include "matrix.hpp"

#include <algorithm>
#include <cassert>
#include <vector>

#ifdef USE_THRUST
#  define HOST __host__
#  define DEVICE __device__
#  include <thrust/copy.h>
#  include <thrust/device_ptr.h>
#  include <thrust/device_vector.h>
#  include <thrust/execution_policy.h>
#  include <thrust/for_each.h>
#  include <thrust/iterator/counting_iterator.h>
#else
#  define HOST
#  define DEVICE
#  include <execution>
#  include <ranges>
#endif

namespace
{
enum class backend : int
{
  STL    = 0,
  Thrust = 1
};

template <backend b>
struct backend_traits;

#ifdef USE_THRUST
template <>
struct backend_traits<backend::Thrust>
{
  template <typename T>
  using host_vector = std::vector<T>;

  template <typename T>
  using device_vector = thrust::device_vector<T>;

  template <typename InputIt, typename OutputIt>
  static void
  copy(InputIt first, InputIt last, OutputIt out)
  {
    thrust::copy(first, last, out);
  }

  template <typename Pointer>
  static auto
  pointer(Pointer ptr)
  {
    return thrust::raw_pointer_cast(ptr);
  }

  template <typename Functor>
  static void
  parallel_for(int total, Functor functor)
  {
    thrust::for_each(thrust::device,
                     thrust::make_counting_iterator(0),
                     thrust::make_counting_iterator(total),
                     functor);
  }
};

constexpr backend selected_backend = backend::Thrust;
#else
template <>
struct backend_traits<backend::STL>
{
  template <typename T>
  using host_vector = std::vector<T>;

  template <typename T>
  using device_vector = std::vector<T>;

  template <typename InputIt, typename OutputIt>
  static void
  copy(InputIt first, InputIt last, OutputIt out)
  {
    std::copy(first, last, out);
  }

  template <typename T>
  static T *
  pointer(T *ptr)
  {
    return ptr;
  }

  template <typename Functor>
  static void
  parallel_for(int total, Functor functor)
  {
    const auto indices = std::views::iota(0, total);
    std::for_each(std::execution::par, indices.begin(), indices.end(), functor);
  }
};

constexpr backend selected_backend = backend::STL;
#endif

using active_backend = backend_traits<selected_backend>;

struct transpose_functor
{
  const double *in;
  double *out;
  int in_rows;
  int out_rows;
  int out_cols;

  HOST DEVICE void
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

  HOST DEVICE void
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

  active_backend::host_vector<double> h_in(data.begin(), data.end());
  active_backend::device_vector<double> d_in(h_in.size());
  active_backend::device_vector<double> d_out(retval.get_rows() * retval.get_cols());

  active_backend::copy(h_in.begin(), h_in.end(), d_in.begin());

  const int total = static_cast<int>(retval.get_rows() * retval.get_cols());
  transpose_functor f{active_backend::pointer(d_in.data()),
                      active_backend::pointer(d_out.data()),
                      static_cast<int>(get_rows()),
                      static_cast<int>(retval.get_rows()),
                      static_cast<int>(retval.get_cols())};

  active_backend::parallel_for(total, f);

  active_backend::copy(d_out.begin(), d_out.end(), retval.get_data());
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

  active_backend::host_vector<double> h_A(A.get_data(), A.get_data() + M * K);
  active_backend::host_vector<double> h_B(B.get_data(), B.get_data() + K * N);
  active_backend::device_vector<double> d_A(h_A.size());
  active_backend::device_vector<double> d_B(h_B.size());
  active_backend::device_vector<double> d_C(M * N, 0.0);

  active_backend::copy(h_A.begin(), h_A.end(), d_A.begin());
  active_backend::copy(h_B.begin(), h_B.end(), d_B.begin());

  const int total = M * N;
  matmul_functor f{
    active_backend::pointer(d_A.data()),
    active_backend::pointer(d_B.data()),
    active_backend::pointer(d_C.data()),
    M,
    N,
    K};

  active_backend::parallel_for(total, f);

  active_backend::copy(d_C.begin(), d_C.end(), retval.get_data());
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
