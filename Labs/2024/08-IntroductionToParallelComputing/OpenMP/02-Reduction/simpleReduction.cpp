#include <omp.h>
#include <iostream>

int main(){

  int sum = 0;
  omp_set_num_threads(4);

#pragma omp parallel for reduction(+:sum)
  for(int i = 0; i < 100; i++){
    sum += i;
  }

  std::cout << "Sum: " << sum << std::endl;

  return 0;
}
