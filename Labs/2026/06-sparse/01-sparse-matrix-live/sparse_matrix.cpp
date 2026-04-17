#include "sparse_matrix.hpp"

void sparse_matrix::init ()
{
  m = 0;
  nnz = 0;
}

void sparse_matrix::update ()
{
  init ();
  for (size_t i = 0; i <size (); ++i)
  {
    if ((*this)[i].size () > 0)
    {
      nnz += (*this)[i].size ();
      
      size_t tmp = (*this)[i].rbegin () -> first + 1; //looping over j
      
      m = std::max (m, tmp);
    }
  }
}

std::ostream &
operator << (std::ostream & os, sparse_matrix &M)
{
  sparse_matrix::col_iter j;
  
  M.update ();
  //os << nnz
  
  os << "matrix = [ ";
  for (size_t i = 0; i < M.size (); ++i)
    if (M[i].size ())
      for (j=M[i].begin (); j!=M[i].end(); ++i)
        {
          os << i+1 <<" , " << M.col_idx(j) + 1 << ", ";
          os << M.col_value (j) << "; " << std::endl;
        }
}

void sparse_matrix::operator += (sparse_matrix & rhs) // A+=B
{
  //some asserts... rows () == rhs.rows()
  
  for (size_t i = 0; i < rhs.size (); ++i)
    if (rhs[i].size())
      for (col_iter j = rhs[i].begin (); j!= rhs[i].end (); ++j)
        (*this)[i][j->first] += rhs.col_value (j);
  
  //re do the asserts
}

std::vector <double>
operator * (sparse_matrix & M, const std::vector <double> & x)
{
  M.update ();
  
  /*
  do some assert, check... compatible dimensions of M, x...
  */
  
  std::vector <double> b (M.rows (), 0.0); // b_i = A_ij x_j  
  sparse_matrix::col_iter j;
  for (size_t i = 0; i < M.size (); ++i)
    if (M[i].size())
      for (j = M[i].begin (); j!= M[i].end (); ++j)
        b[i] += M.col_value (j) * x [M.col_idx (j)];
  return b;
}


// Convert row−oriented sparse matrix to CSR format .
void sparse_matrix::csr (std::vector <double> &a,
              std::vector <int> &col_ind,
              std::vector <int> &row_ptr)
{
  int idx = 0, idr = 1;
  sparse_matrix :: col_iter j;
  
  for (size_t i = 0; i < size (); ++i)
  {
    if ((*this)[i].size ())
    {
      for (j = (*this)[i].begin (); j != (*this)[i].end (); ++j)
        {
          col_ind[idx] = col_idx (j);
          a[idx] = col_value (j);
          ++idx;
        }
    }
    row_ptr[idr++] = idx; //++idr != idr++
  }
}






















