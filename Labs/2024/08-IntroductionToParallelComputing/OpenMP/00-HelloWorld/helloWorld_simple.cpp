#include <omp.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

    #pragma omp parallel num_threads(8)
    {
        printf("Hello, World! \n");
    }
    return 0;
}
