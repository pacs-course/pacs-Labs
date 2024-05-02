// Compile without the flag -O3 in the compiler
// otherwise the compiler will have tid private.

#include <stdio.h>
#include <omp.h>

int main(){
  int tid = 0;
  omp_set_num_threads(4);

#pragma omp parallel
  {
    tid = omp_get_thread_num();
    for (int i = 0; i < 1e5; i++){};
    printf("Thread %d\n", tid);
  }

return 0;
}
