#include "matrix.hpp"

#include <cblas.h>

#include <Eigen/Dense>
#include <algorithm>
#include <cassert>

matrix matrix::transpose() const {
  matrix result(get_cols(), get_rows());

  for (size_t i = 0; i < result.get_rows(); ++i) {
    for (size_t j = 0; j < result.get_cols(); ++j) {
      result(i, j) = value(j, i);
    }
  }

  return result;
}

matrix operator*(const matrix &A, const matrix &B) {
  assert(A.get_cols() == B.get_rows());

  matrix result(A.get_rows(), B.get_cols());

  for (size_t i = 0; i < result.get_rows(); ++i)
    for (size_t j = 0; j < result.get_cols(); ++j)
      for (size_t k = 0; k < A.get_cols(); ++k)
        result(i, j) += A(i, k) * B(k, j);

  return result;
}

matrix matrix::mmult_transpose_trick(const matrix &other) {
  assert(get_cols() == other.get_rows());

  matrix result(get_rows(), other.get_cols());

  matrix tmp = transpose();

  for (size_t i = 0; i < result.get_rows(); ++i)
    for (size_t j = 0; j < result.get_cols(); ++j)
      for (size_t k = 0; k < get_cols(); ++k)
        result(i, j) += tmp(k, i) * other(k, j);

  return result;
}

matrix matrix::mmult_eigen(const matrix &other) {
  assert(get_cols() == other.get_rows());
  matrix result(get_rows(), other.get_cols());

  Eigen::Map<const Eigen::MatrixXd> eigen_A(get_data(), get_rows(), get_cols());
  Eigen::Map<const Eigen::MatrixXd> eigen_B(other.get_data(), other.get_rows(),
                                            other.get_cols());
  Eigen::Map<Eigen::MatrixXd> eigen_result(result.get_data(), result.get_rows(),
                                           result.get_cols());
  eigen_result = eigen_A * eigen_B;
  return result;
}

matrix matrix::mmult_openblas(const matrix &other) {
  assert(get_cols() == other.get_rows());
  matrix result(get_rows(), other.get_cols());
  cblas_dgemm(
    CblasColMajor,     // order
    CblasNoTrans,      // transpose A
    CblasNoTrans,      // transpose B
    get_rows(),        // Number of rows in matrices A and C. 
    other.get_cols(),  // Number of columns in matrices B and C.
    get_cols(),        // Number of columns in matrix A; number of rows in matrix B.
    1.0,               // Scaling factor for the product of matrices A and B.
    get_data(),        // A
    get_rows(),        // The size of the first dimension of matrix A
    other.get_data(),  // B
    other.get_rows(),  // The size of the first dimension of matrix B
    1.0,               // Scaling factor for matrix C.
    result.get_data(), // C
    result.get_rows()  // The size of the first dimension of matrix C
  );
  return result;
}

void matrix::solve(matrix &rhs) {
  size_t ii, jj, kk;
  double f;

  // Factorize
  if (!factorized) {
    factorize();
    std::cout << "factorize !" << std::endl;
  }

  double *b = rhs.get_data();
  // Do Forward Substitution
  std::cout << "fwdsub !" << std::endl;
  for (ii = 0; ii < get_rows(); ++ii) {
    f = b[m_p[ii]];
    for (kk = 0; kk < ii; ++kk) {
      f -= value(m_p[ii], kk) * b[m_p[kk]];
    }
    b[m_p[ii]] = f;
  }

  // Do Backward Substitution
  std::cout << "bwdsub !" << std::endl;
  for (jj = 1; jj <= get_rows(); ++jj) {
    ii = get_rows() - jj;
    f = b[m_p[ii]];
    for (kk = ii + 1; kk < get_cols(); ++kk) {
      f -= value(m_p[ii], kk) * b[m_p[kk]];
    }
    b[m_p[ii]] = f / value(m_p[ii], ii);
  }
}

void matrix::factorize() {
  m_p.resize(m_rows, 0);
  for (size_t ii = 0; ii < m_rows; ++ii) {
    m_p[ii] = ii;
  }

  size_t m = this->get_rows();
  size_t n = this->get_cols();
  size_t ii, jj, kk;

  double pivot = 0., maxpivot = 0.;

  size_t imaxpivot = 0;

  assert(m == n);
  for (ii = 0; ii < m - 1; ++ii) {
    maxpivot = value(m_p[ii], ii);
    imaxpivot = ii;
    for (kk = ii + 1; kk < m; ++kk)
      if (value(m_p[kk], ii) > maxpivot) {
        maxpivot = value(m_p[kk], ii);
        imaxpivot = kk;
      }

    if (imaxpivot != ii) {
      std::swap(m_p[ii], m_p[imaxpivot]);
    }

    pivot = value(m_p[ii], ii);
    for (jj = ii + 1; jj < m; ++jj) {
      value(m_p[jj], ii) = value(m_p[jj], ii) / pivot;

      for (kk = ii + 1; kk < n; ++kk)
        value(m_p[jj], kk) += -value(m_p[ii], kk) * value(m_p[jj], ii);
    }
  }
  factorized = true;
}
