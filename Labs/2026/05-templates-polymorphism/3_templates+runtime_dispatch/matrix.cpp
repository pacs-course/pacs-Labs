#include "matrix.hpp"

#include <algorithm>
#include <cassert>
#include <execution>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <thrust/copy.h>
#include <thrust/device_ptr.h>
#include <thrust/device_vector.h>
#include <thrust/execution_policy.h>
#include <thrust/for_each.h>
#include <thrust/iterator/counting_iterator.h>
#include <vector>

#define HOST __host__
#define DEVICE __device__

namespace runtime_portability_demo
{
enum class backend : int
{
  STL    = 0,
  Thrust = 1
};

backend
parse_backend(std::string_view name)
{
  if (name == "stl")
    return backend::STL;
  if (name == "thrust")
    return backend::Thrust;

  throw std::invalid_argument("backend must be 'stl' or 'thrust'");
}
} // namespace runtime_portability_demo

namespace
{
struct transpose_functor
{
  const double *in;
  double *out;
  int in_rows;
  int out_rows;

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

namespace stl_backend
{
template <typename Functor>
void
parallel_for(int total, Functor functor)
{
  const auto indices = std::views::iota(0, total);
  std::for_each(std::execution::par, indices.begin(), indices.end(), functor);
}

matrix
transpose_impl(const matrix &input)
{
  matrix retval(input.get_cols(), input.get_rows());

  std::vector<double> in(input.get_data(), input.get_data() + input.get_rows() * input.get_cols());
  std::vector<double> out(retval.get_rows() * retval.get_cols());

  const int total = static_cast<int>(retval.get_rows() * retval.get_cols());
  transpose_functor f{in.data(),
                      out.data(),
                      static_cast<int>(input.get_rows()),
                      static_cast<int>(retval.get_rows())};

  parallel_for(total, f);
  std::copy(out.begin(), out.end(), retval.get_data());
  return retval;
}

matrix
multiply_impl(const matrix &A, const matrix &B)
{
  const int M = static_cast<int>(A.get_rows());
  const int N = static_cast<int>(B.get_cols());
  const int K = static_cast<int>(A.get_cols());
  assert(K == static_cast<int>(B.get_rows()));

  matrix retval(M, N);

  std::vector<double> a(A.get_data(), A.get_data() + M * K);
  std::vector<double> b(B.get_data(), B.get_data() + K * N);
  std::vector<double> c(M * N, 0.0);

  const int total = M * N;
  matmul_functor f{a.data(), b.data(), c.data(), M, K};

  parallel_for(total, f);
  std::copy(c.begin(), c.end(), retval.get_data());
  return retval;
}
} // namespace stl_backend

namespace thrust_backend
{
template <typename Functor>
void
parallel_for(int total, Functor functor)
{
  thrust::for_each(thrust::device,
                   thrust::make_counting_iterator(0),
                   thrust::make_counting_iterator(total),
                   functor);
}

matrix
transpose_impl(const matrix &input)
{
  matrix retval(input.get_cols(), input.get_rows());

  std::vector<double>           h_in(input.get_data(), input.get_data() + input.get_rows() * input.get_cols());
  thrust::device_vector<double> d_in(h_in.size());
  thrust::device_vector<double> d_out(retval.get_rows() * retval.get_cols());

  thrust::copy(h_in.begin(), h_in.end(), d_in.begin());

  const int total = static_cast<int>(retval.get_rows() * retval.get_cols());
  transpose_functor f{thrust::raw_pointer_cast(d_in.data()),
                      thrust::raw_pointer_cast(d_out.data()),
                      static_cast<int>(input.get_rows()),
                      static_cast<int>(retval.get_rows())};

  parallel_for(total, f);
  thrust::copy(d_out.begin(), d_out.end(), retval.get_data());
  return retval;
}

matrix
multiply_impl(const matrix &A, const matrix &B)
{
  const int M = static_cast<int>(A.get_rows());
  const int N = static_cast<int>(B.get_cols());
  const int K = static_cast<int>(A.get_cols());
  assert(K == static_cast<int>(B.get_rows()));

  matrix retval(M, N);

  std::vector<double>           h_A(A.get_data(), A.get_data() + M * K);
  std::vector<double>           h_B(B.get_data(), B.get_data() + K * N);
  thrust::device_vector<double> d_A(h_A.size());
  thrust::device_vector<double> d_B(h_B.size());
  thrust::device_vector<double> d_C(M * N, 0.0);

  thrust::copy(h_A.begin(), h_A.end(), d_A.begin());
  thrust::copy(h_B.begin(), h_B.end(), d_B.begin());

  const int total = M * N;
  matmul_functor f{thrust::raw_pointer_cast(d_A.data()),
                   thrust::raw_pointer_cast(d_B.data()),
                   thrust::raw_pointer_cast(d_C.data()),
                   M,
                   K};

  parallel_for(total, f);
  thrust::copy(d_C.begin(), d_C.end(), retval.get_data());
  return retval;
}
} // namespace thrust_backend
} // namespace

namespace runtime_portability_demo
{
matrix
transpose(const matrix &input, backend selected_backend)
{
  switch (selected_backend)
    {
    case backend::STL:
      return stl_backend::transpose_impl(input);
    case backend::Thrust:
      return thrust_backend::transpose_impl(input);
    }

  throw std::invalid_argument("unsupported backend");
}

matrix
multiply(const matrix &A, const matrix &B, backend selected_backend)
{
  switch (selected_backend)
    {
    case backend::STL:
      return stl_backend::multiply_impl(A, B);
    case backend::Thrust:
      return thrust_backend::multiply_impl(A, B);
    }

  throw std::invalid_argument("unsupported backend");
}
} // namespace runtime_portability_demo
