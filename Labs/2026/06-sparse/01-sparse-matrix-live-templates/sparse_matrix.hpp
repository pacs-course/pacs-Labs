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
template <typename T> //T=double
class sparse_matrix : public std::vector <std::map <int, T>>
{
  private:
    using col_type = std::map <int, T>;
    using row_type = std::vector <col_type>;

    size_t m;
    size_t nnz;
  
    void init ();
    void update ();
  
  public:
    using row_iter = typename row_type :: iterator; //try to comment-out typename
    using col_iter = typename col_type :: iterator;
    
    sparse_matrix () {init ();}
    
    inline int
    col_idx (const col_iter & j) const {return j -> first;}
    
    inline T
    col_value (const col_iter & j) const {return j -> second;}
    
    inline size_t rows () const {return this->size ();}
    
    inline size_t cols () const {return m;}
    
    void csr (std::vector <T> &a,
              std::vector <int> &col_ind,
              std::vector <int> &row_ptr);
    
    void operator += (sparse_matrix &rhs); // a+=a
    
    // b = A*x
    template <typename Y> friend std::vector <Y> 
    operator * (sparse_matrix<Y> & M, const std::vector <Y> &x);
    
    template <typename Y> friend std::ostream & // std::cout << A << std::endl 
    operator << (std::ostream & os, sparse_matrix<Y> & M);
};

#include "sparse_matrix_impl.hpp"








