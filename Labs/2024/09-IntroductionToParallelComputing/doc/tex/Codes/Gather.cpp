#include <iostream>
#include <mpi.h>


int main( int argv, char* argc[])
{
    MPI_Init(&argv, &argc);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int sendCount = 2;          // "global" variable

    // example with array
    int sendData[sendCount] = {rank * 2, rank * 2 + 1};

    int *recvData = nullptr;

    if (rank == 0) {
        // Allocate memory for the gathered data at the root process
        recvData = new int[sendCount * size];
    }

    MPI_Gather(sendData, sendCount, MPI_INT, recvData, sendCount, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        // Display the gathered data at the root process
        std::cout << "Root process [0] gathered data: ";
        for (int i = 0; i < size * sendCount; ++i) {
            std::cout << recvData[i] << " ";
        }
        std::cout << std::endl;

        delete[] recvData;
    }


    MPI_Finalize();
    return 0;
}
