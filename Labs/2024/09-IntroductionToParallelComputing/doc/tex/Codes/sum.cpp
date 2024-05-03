#include <mpi.h>
#include <iostream>
#include <vector>

int main(int argv, char* argc[])
{
    MPI_Init(&argv, &argc);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    std::vector<int> data;
    std::vector<int> recvdata;

    int dataSize;

    if (rank == 0)
    {
        int dataTmp = 0;
        std::cout << "Inserting data..." << std::endl;
        while (std::cin >> dataTmp){
            data.push_back(dataTmp);
        }
        dataSize = data.size();
    }

    MPI_Bcast( &dataSize, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int dataChunk = ((dataSize % size > rank) ? (dataSize/size + 1) : dataSize/size);

    recvdata.resize(dataChunk);

    MPI_Scatterv( data.data(), dataChunk, MPI_INT, recvdata.data() , dataChunk, MPI_INT, 0, MPI_COMM_WORLD);

    std::cout <<"rank:" << rank << " data:" <<  dataChunk << std::endl;

    std::cout << "Priting data " << std::endl;
    for (int i=0; i < dataChunk; i++){
        std::cout << "["<< rank << "] " << recvdata[i] << std::endl;
    }


    MPI_Finalize();
}
