#include <omp.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

  omp_set_num_threads(8);

  double tic = omp_get_wtime();

#pragma omp parallel
  {
      int a =  omp_get_thread_num();
      printf("Current thread: %i \n", a);
      printf("Hello, World! \n");
  }

  printf("\n");
  printf("\n");
  printf("Next run is going to be with 4 threads \n");
  printf("\n");
  printf("\n");

  omp_set_num_threads(4);

#pragma omp parallel
  {
      int a =  omp_get_thread_num();
      printf("Current thread: %i \n", a);
      printf("Hello, World! \n");
  }

  double toc = omp_get_wtime();

  printf("Elapsed time: %f seconds \n", toc - tic);

  return 0;
}
