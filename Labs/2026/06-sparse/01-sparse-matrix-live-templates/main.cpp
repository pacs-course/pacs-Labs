#include <sparse_matrix.hpp>

#include <iostream>
#include <vector>

using number_type = double;
int main ()
{
  const int n = 10;
  
  //A * x   A_ij x_j
  
  std::vector <number_type> x (n,1.0);
  
  sparse_matrix<number_type> A;
  A.resize (n);//resize, ....inheriting from STL
  
  for (size_t i = 0; i < A.rows (); ++i)
  {
    if (i>0)
      A[i][i-1] = -1;
    if (i<n-1)
      A[i][i+1] = -1;
    A[i][i] =4;
  }
  
  std::cout << A << std::endl;
  
  std::cout << "x = \n";
  for (const auto & r: x)
    std::cout << r << "\n";
  std::cout << std::endl;
  
  std::vector <number_type> b = A*x;
  std::cout << "A * x = \n";
  for (const auto & r: b)
    std::cout << r << "\n";
  std::cout << std::endl;
  
  return 0;
  
}