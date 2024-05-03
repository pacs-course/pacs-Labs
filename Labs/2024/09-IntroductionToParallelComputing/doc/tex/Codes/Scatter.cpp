#include <iostream>
#include <mpi.h>


int main( int argv, char* argc[])
{
    MPI_Init(&argv, &argc);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int recvCount = 2;
    int recvBuffer[2];

    int* sendBuffer = NULL;

    if (rank == 0)
    {
        sendBuffer = (int*) malloc( sizeof(int) * size * 2);
        for (int i=0; i < 2*size; i++){
            sendBuffer[i] = i;
        }
    }

    MPI_Scatter(sendBuffer, recvCount, MPI_INT, 
                recvBuffer, recvCount, MPI_INT, 0, MPI_COMM_WORLD);

    std::cout << "rank " << rank << " \nData:" << recvBuffer[0] << " " << recvBuffer[1] << std::endl;

    MPI_Finalize();
    return 0;
}
