#include "sparse_matrix.hpp"
// You may want to speedup the compilation when T -double
template class SparseMatrix<double>;
template std::vector<double> operator*(SparseMatrix<double> &M, const std::vector<double> &x);
template std::ostream &operator<<(std::ostream &stream, SparseMatrix<double> &M);
