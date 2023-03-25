#ifndef HH_SPARSE_MATRIX_HPP_HH
#define HH_SPARSE_MATRIX_HPP_HH

#include "sparse_matrix.hpp"
#include <iomanip>
#include <cassert>

template <class T>
void
SparseMatrix<T>::update_properties()
{
  m_nnz = 0;
  m_m   = 0;

  for (size_t i = 0; i < this->size(); ++i)
    {
      if (!(*this)[i].empty())
        {
          m_nnz += (*this)[i].size();

          // std::map are sorted, so we just need to get the index of
          // the last element.
          size_t tmp = (*this)[i].rbegin()->first + 1;
          m_m = std::max(m_m, tmp);
        }
    }
}


template <class T>
std::ostream &
operator<<(std::ostream &stream, SparseMatrix<T> &M)
{
  M.update_properties();
  stream << "nrows = " << M.size() << "; ncols = " << M.m_m
         << "; nnz = " << M.m_nnz << ";" << std::endl;
  stream << "mat = [";
  for (size_t i = 0; i < M.size(); ++i)
    {
      for (auto j = M[i].begin(); j != M[i].end(); ++j)
        {
          stream << i + 1 << ", " << j->first + 1 << ", ";
          stream << std::setprecision(16) << j->second << ";" << std::endl;
        }
    }
  stream << "];" << std::endl;
  return stream;
}

template <class T>
void
SparseMatrix<T>::aij(std::vector<T>            &a,
                     std::vector<unsigned int> &i,
                     std::vector<unsigned int> &j)
{
  update_properties();
  a.resize(m_nnz);
  i.resize(m_nnz);
  j.resize(m_nnz);

  unsigned int idx = 0;

  for (size_t ii = 0; ii < this->size(); ++ii)
    for (auto jj = (*this)[ii].begin(); jj != (*this)[ii].end(); ++jj)
      {
        i[idx] = ii;
        j[idx] = jj->first;
        a[idx] = jj->second;
        ++idx;
      }
}

template <class T>
void
SparseMatrix<T>::csr(std::vector<T>            &val,
                     std::vector<unsigned int> &col_ind,
                     std::vector<unsigned int> &row_ptr)
{
  update_properties();
  val.resize(m_nnz);
  col_ind.resize(m_nnz);
  row_ptr.resize(size() + 1);
  row_ptr[0] = 0;

  unsigned int idx = 0;

  for (size_t ii = 0; ii < this->size(); ++ii)
    {
      for (auto jj = (*this)[ii].begin(); jj != (*this)[ii].end(); ++jj)
        {
          col_ind[idx] = jj->first;
          val[idx]     = jj->second;
          ++idx;
        }

      row_ptr[ii + 1] = idx;

      // Or, equivalently:
      // row_ptr[ii + 1] = row_ptr[ii] + (*this)[ii].size();
    }
}

template <class T>
void
SparseMatrix<T>::operator+=(SparseMatrix<T> &other)
{
  assert(size() == other.size());
  assert(m_m == other.m_m);

  for (size_t ii = 0; ii < other.size(); ++ii)
    for (auto jj = other[ii].begin(); jj != other[ii].end(); ++jj)
      (*this)[ii][jj->first] += jj->second;
}

template <class T>
std::vector<T>
operator*(SparseMatrix<T> &M, const std::vector<T> &x)
{
  std::vector<T> y(M.size());

  for (unsigned int i = 0; i < M.size(); ++i)
    for (auto j = M[i].begin(); j != M[i].end(); ++j)
      y[i] += j->second * x[j->first];
  return y;
}

#endif
