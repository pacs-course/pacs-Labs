#include <omp.h>
#include <iostream>

int main(){
  int a = 0;
#pragma omp parallel 
  {
    a = a + omp_get_thread_num();
  }
  std::cout << a << std::endl;
  return 0;
}
