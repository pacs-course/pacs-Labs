#ifndef HAVE_MATRIX_H
#define HAVE_MATRIX_H

#include <iostream>
#include <vector>

class matrix {
 public:
  matrix(size_t size) : m_rows(size), m_cols(size) {
    m_data.resize(m_rows * m_cols, 0.0);
  };

  matrix(size_t rows_, size_t cols_) : m_rows(rows_), m_cols(cols_) {
    m_data.resize(m_rows * m_cols, 0.0);
  };

  matrix(const matrix &) = default;

  size_t get_rows() const { return m_rows; }

  size_t get_cols() const { return m_cols; }

  double &operator()(size_t irow, size_t jcol) { return value(irow, jcol); };

  const double &operator()(size_t irow, size_t jcol) const {
    return value(irow, jcol);
  };

  const double *get_data() const { return &(m_data[0]); };

  double *get_data() { return &(m_data[0]); };

  /// transposed matrix : B = A'
  matrix transpose() const;

  matrix mmult_transpose_trick(const matrix &other);
  matrix mmult_eigen(const matrix &other);

  void solve(matrix &rhs);

  void factorize();

 private:
  std::vector<double> m_data;
  std::vector<int> m_p;
  const size_t m_rows;
  const size_t m_cols;

  inline size_t sub2ind(const size_t ir, const size_t jc) const {
    return (ir + jc * m_rows);
  };

  double &value(size_t irow, size_t jcol) {
    return m_data[sub2ind(irow, jcol)];
  };

  const double &value(size_t irow, size_t jcol) const {
    return m_data[sub2ind(irow, jcol)];
  };

  bool factorized;
};

/// matrix x matrix product : C = A * B
matrix operator*(const matrix &A, const matrix &B);

#endif
