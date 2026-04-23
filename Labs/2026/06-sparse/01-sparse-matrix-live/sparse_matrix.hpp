#pragma once

#include <iostream>
#include <vector>
#include <map>
/*
A[i] i-th row
A[i][j] j-th element of the i-th row

let's consider a matrix as a 
std::vector <std::map <int, double>>
A[i] i-th row --> vector
A[i][j] map


*/

class sparse_matrix : public std::vector <std::map <int, double>>
{
  private:
    using col_type = std::map <int, double>;
    using row_type = std::vector <col_type>;

    size_t m;
    size_t nnz;
  
    void init ();
    void update ();
  
  public:
    using row_iter = row_type :: iterator;
    using col_iter = col_type :: iterator;
    
    sparse_matrix () {init ();}
    
    inline int
    col_idx (const col_iter & j) const {return j -> first;}
    
    inline double
    col_value (const col_iter & j) const {return j -> second;}
    
    inline size_t rows () const {return size ();}
    
    inline size_t cols () const {return m;}
    
    void csr (std::vector <double> &a,
              std::vector <int> &col_ind,
              std::vector <int> &row_ptr);
    
    void operator += (sparse_matrix &rhs); // a+=a
    
    // b = A*x
    friend std::vector <double> 
    operator * (sparse_matrix & M, const std::vector <double> &x);
    
    friend std::ostream & // std::cout << A << std::endl 
    operator << (std::ostream & os, sparse_matrix & M);
};









