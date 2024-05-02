// firstprivate
#include <iostream>
#include <omp.h>

int main(){
  int k = 100;

  #pragma omp parallel firstprivate(k)
  {
    k = k + omp_get_thread_num();
    std::cout << "k = " << k << std::endl;
  } 

  std::cout << "After parallel run:\nk = " << k << std::endl;

  return 0;

}
