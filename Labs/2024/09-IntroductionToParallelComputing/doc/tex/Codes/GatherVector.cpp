#include <iostream>
#include <mpi.h>
#include <vector>


int main( int argv, char* argc[])
{
    MPI_Init(&argv, &argc);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int sendCount = 2;          // "global" variable

    // example with array
    // int sendData[sendCount] = {rank * 2, rank * 2 + 1};
    std::vector<int> sendData;

    for(int i=0; i < sendCount; i++){
        sendData.push_back(rank*sendCount + i);
    }

    //int *recvData = nullptr;
    std::vector<int> recvData;

    if (rank == 0) {
        // Allocate memory for the gathered data at the root process
        // recvData = new int[sendCount * size];
        recvData.resize(sendCount * size);
    }

     // Perform MPI_Gather to gather data from all processes to the root process
    MPI_Gather(sendData.data(), sendCount, 
                    MPI_INT, recvData.data(), sendCount, MPI_INT, 0, MPI_COMM_WORLD);



    if (rank == 0) {
        // Display the gathered data at the root process
        std::cout << "Root process [0] gathered data: ";
        for (int i = 0; i < size * sendCount; ++i) {
            std::cout << recvData[i] << " ";
        }
        std::cout << std::endl;

        // Clean up the allocated memory
        // delete[] recvData;
    }


    MPI_Finalize();
    return 0;
}
