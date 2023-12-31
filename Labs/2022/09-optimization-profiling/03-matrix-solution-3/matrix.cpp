#include "matrix.hpp"

#include <algorithm>
#include <cassert>

#include <Eigen/Dense>

matrix
matrix::transpose() const
{
  matrix result(*this);

  Eigen::Map<Eigen::MatrixXd> eigen_result(result.get_data(),
                                           result.get_rows(),
                                           result.get_cols());

  eigen_result.transposeInPlace();

  return result;
}

matrix
operator*(const matrix &A, const matrix &B)
{
  assert(A.get_cols() == B.get_rows());

  Eigen::Map<const Eigen::MatrixXd> eigen_A(A.get_data(),
                                            A.get_rows(),
                                            A.get_cols());

  Eigen::Map<const Eigen::MatrixXd> eigen_B(B.get_data(),
                                            B.get_rows(),
                                            B.get_cols());

  matrix result(A.get_rows(), B.get_cols());

  Eigen::Map<Eigen::MatrixXd> eigen_result(result.get_data(),
                                           result.get_rows(),
                                           result.get_cols());

  eigen_result = eigen_A * eigen_B;

  return result;
}

void
matrix::solve(matrix &rhs)
{
  size_t ii, jj, kk;
  double f;

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
  for (size_t ii = 0; ii < rows; ++ii)
    p[ii] = ii;

  size_t m = this->get_rows();
  size_t n = this->get_cols();
  size_t ii, jj, kk;

  double pivot = 0., maxpivot = 0.;

  size_t imaxpivot = 0;

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
