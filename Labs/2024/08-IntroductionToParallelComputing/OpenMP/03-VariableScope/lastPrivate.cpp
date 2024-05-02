// lastprivate
#include <iostream>
#include <omp.h>

int main(){
  int k = 100;

  #pragma omp parallel for lastprivate(k)
  for(int i = 0; i < 20; i++){
    k = i;
  }

  std::cout << "After parallel run:\nk = " << k << std::endl;

  return 0;

}
