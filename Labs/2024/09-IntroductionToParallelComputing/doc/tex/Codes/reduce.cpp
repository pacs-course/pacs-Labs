#include <iostream>
#include <mpi.h>

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double a[2], red[2];
    red[0] = 0.0; 
    red[1] = 0.0;
    a[0] = 1.0;     // everyone knows everything
    a[1] = 2.0;

    int root = 2;
    MPI_Reduce(&a, &red, 2, MPI_DOUBLE, MPI_SUM, root, MPI_COMM_WORLD);

    std::cout << "["<< rank << "] " <<   a[0] << " " <<   a[1] << std::endl;
    std::cout << "["<< rank << "] " << red[0] << " " << red[1] << std::endl;

    MPI_Finalize();
}
