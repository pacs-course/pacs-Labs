#include <iostream>
#include <mpi.h>

int main(int argc, char *argv[]) {

    MPI_Init(&argc, &argv);

    int rank,size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

//    std::cout << "["<< rank << "] "<<"Hello, World!" << std::endl;
    // write a sorted output for helloworld
    char message[100];

    if (rank != 0){
      sprintf(message, "Hello, World! from process %d", rank);
      MPI_Send(message, 100, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }
    else{

      sprintf(message, "Hello, World! from process %d", rank);
      std::cout << message << std::endl;

      for (int i = 1; i < size; i++){
        MPI_Recv(message, 100, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << message << std::endl;
      }
    }

    MPI_Finalize();

    return 0;
}
