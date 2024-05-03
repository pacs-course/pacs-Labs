#include <iostream>
#include <mpi.h>

int main(int argc, char* argv[])
{
    int rank, size;
    double a[2];
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if ( rank == 0)     // we populate a
    {
        a[0] = 2.0;     
        a[1] = 1.0;
    }

    MPI_Bcast(a, 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    std::cout << a[0] << " " << a[1] << std::endl;

    MPI_Finalize();
}
