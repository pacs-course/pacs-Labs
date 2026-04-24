template <typename T>
void sparse_matrix<T>::init ()
{
  m = 0;
  nnz = 0;
}

template <typename T>
void sparse_matrix<T>::update ()
{
  init ();
  for (size_t i = 0; i <rows (); ++i)
  {
    if ((*this)[i].size () > 0)
    {
      nnz += (*this)[i].size ();
      
      size_t tmp = (*this)[i].rbegin () -> first + 1; //looping over j
      
      m = std::max (m, tmp);
    }
  }
}

template <typename T> std::ostream &
operator << (std::ostream & os, sparse_matrix<T> &M)
{
  typename sparse_matrix<T>::col_iter j;
  
  M.update ();
  //os << nnz
  
  os << "matrix = [ ";
  for (size_t i = 0; i < M.size (); ++i)
    if (M[i].size ())
      for (j = M[i].begin (); j != M[i].end (); ++j)
        {
          os << i+1 <<" , " << M.col_idx(j) + 1 << ", ";
          os << M.col_value (j) << ";\n ";
        }
  os << "]" << std::endl;
  return os;
}

template <typename T>
void sparse_matrix<T>::operator += (sparse_matrix<T> & rhs) // A+=B
{
  //some asserts... rows () == rhs.rows()
  
  for (size_t i = 0; i < rhs.size (); ++i)
    if (rhs[i].size())
      for (typename sparse_matrix<T>::col_iter j = rhs[i].begin (); j!= rhs[i].end (); ++j)
        (*this)[i][j->first] += rhs.col_value (j);
  
  //re do the asserts
}

template <typename T> std::vector <T>
operator * (sparse_matrix<T> & M, const std::vector <T> & x)
{
  M.update ();
  
  /*
  do some assert, check... compatible dimensions of M, x...
  */
  
  std::vector <T> b (M.rows (), 0.0); // b_i = A_ij x_j  
  typename sparse_matrix<T>::col_iter j;
  for (size_t i = 0; i < M.size (); ++i)
    if (M[i].size())
      for (j = M[i].begin (); j!= M[i].end (); ++j)
        b[i] += M.col_value (j) * x [M.col_idx (j)];
  return b;
}


// Convert row−oriented sparse matrix to CSR format .
template <typename T>
void sparse_matrix<T>::csr (std::vector <T> &a,
              std::vector <int> &col_ind,
              std::vector <int> &row_ptr)
{
  int idx = 0, idr = 1;
  typename sparse_matrix<T> :: col_iter j;
  
  for (size_t i = 0; i < this -> size (); ++i)
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





















