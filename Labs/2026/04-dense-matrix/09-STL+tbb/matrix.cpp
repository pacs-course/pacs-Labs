#include "matrix.hpp"

#include <algorithm>
#include <cassert>
#include <execution>
#include <ranges>

matrix
matrix::transpose() const
{
  matrix retval(get_cols(), get_rows());

  const int total = static_cast<int>(retval.get_rows() * retval.get_cols());
  const auto indices = std::views::iota(0, total);
  const double *in = get_data();
  double *out = retval.get_data();
  const int in_rows = static_cast<int>(get_rows());
  const int out_rows = static_cast<int>(retval.get_rows());

  std::for_each(std::execution::par, indices.begin(), indices.end(), [&](int idx) {
    const int i = idx % out_rows;
    const int j = idx / out_rows;
    out[retval.sub2ind(i, j)] = in[sub2ind(j, i)];
  });
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

  const int total = M * N;
  const auto indices = std::views::iota(0, total);
  const double *a = A.get_data();
  const double *b = B.get_data();
  double *c = retval.get_data();

  std::for_each(std::execution::par, indices.begin(), indices.end(), [&](int idx) {
    const int row = idx % M;
    const int col = idx / M;

    double sum = 0.0;
    for (int kk = 0; kk < K; ++kk)
      sum += a[row + kk * M] * b[kk + col * K];

    c[row + col * M] = sum;
  });
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
