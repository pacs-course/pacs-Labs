#include <iostream>
#include <mpi.h>
#include <vector>


int main( int argv, char* argc[])
{
    MPI_Init(&argv, &argc);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // omitting the usual stuff...
    int recvCount = 2;

    std::vector<int> sendBuffer;
    std::vector<int> recvBuffer;

    if (rank == 0)
    {
        for (int i=0; i < 2*size; i++){
            sendBuffer.push_back(i);
        }
    }

    recvBuffer.resize(recvCount);
    MPI_Scatter(sendBuffer.data(), recvCount, MPI_INT, 
                recvBuffer.data(), recvCount, MPI_INT, 0, MPI_COMM_WORLD);

    std::cout << "rank " << rank << " \nData:" << recvBuffer[0] << " " << recvBuffer[1] << std::endl;
    // omitting the usual stuff...

    MPI_Finalize();
    return 0;
}
