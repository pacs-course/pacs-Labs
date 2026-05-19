#include <omp.h>

#include <iostream>

int main ()
{
  int n_threads;
  int thread_id;

  int a[5];
  int b[5];
  
  for (int i = 0; i<5; ++i)
  {
    a[i] = i;
    b[i] = 2;
  }
  
  std::cout << "serial\n\n";
  
  //checksum = \sum_0^{N-1}
  int serial_checksum = 0;
  for (int i = 0; i < 4; ++i)
  {
    a[i] = a[i+1] + b[i]; //loop dependency
    std::cout << a[i] << std::endl;
    serial_checksum += a[i];
  }
  std::cout << "\n" << serial_checksum << std::endl;
  
  for (int i = 0; i<5; ++i)
  {
    a[i] = i;
    b[i] = 2;
  }
  
#pragma omp parallel shared(a, b, n_threads)
  {
  #pragma omp master
    {
    n_threads = omp_get_num_threads ();
    std::cout << "\n\n parallel version -- nt: " 
              << n_threads << std::endl;
    }
  #pragma omp barrier
  
  #pragma omp for
    for (int i = 0; i < 4; ++i)
    {
      a[i] = a[i+1] + b[i]; //loop dependency
      #pragma omp critical
      std::cout << a[i] << std::endl;
    }
  }
  int parallel_checksum = 0;
  #pragma omp parallel for reduction (+ : parallel_checksum)
    for (int i = 0; i < 4; ++i)
      parallel_checksum += a[i];
  
  std::cout << "\n" << parallel_checksum << std::endl;
  
  return 0;
}