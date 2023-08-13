#include "matrix.hpp"

#include <algorithm>
#include <cassert>

matrix matrix::transpose() const {
  std::cout << "WARNING: transpose not implemented" << std::endl;
  return matrix(0);
}

matrix operator*(const matrix &A, const matrix &B) {
  std::cout << "WARNING: operator* not implemented" << std::endl;
  return matrix(0);
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
