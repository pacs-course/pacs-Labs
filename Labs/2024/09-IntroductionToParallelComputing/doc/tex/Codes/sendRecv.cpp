#include <iostream>
#include <mpi.h>

int main(int argc, char* argv[])
{
    int rank, size;
    double a[2];
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if ( rank == 0)
    {
        a[0] = 2.0;     
        a[1] = 1.0;
        for (int i=0; i < size; i++)
        {
            MPI_Send(a, 2, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(a, 2, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    std::cout << a[0] << " " << a[1] << std::endl;

    MPI_Finalize();
}
