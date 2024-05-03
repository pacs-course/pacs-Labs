#include <iostream>
#include <mpi.h>

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int sendCount = 2;
    int sendData[2] = { 2*rank, 2*rank+1 };

    int* recvData = nullptr;

    recvData = new int[sendCount * size];

    MPI_Allgather(sendData, sendCount, MPI_INT,
                  recvData, sendCount, MPI_INT, MPI_COMM_WORLD);

    std::cout << "Out from processor " << rank << std::endl;
    for (int i =0; i < 2*size; i++)
    {   
        std::cout << recvData[i] << " ";
    }
    std::cout << std::endl;


    MPI_Finalize();
}
