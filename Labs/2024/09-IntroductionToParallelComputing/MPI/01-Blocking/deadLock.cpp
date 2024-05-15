#include <mpi.h>
#include <vector>
#include <iostream>

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<MPI_Request> requests(size);

    int data;

    if (rank == 0) {
        data = 42;
        for (int i=0; i < 9; i++){
          MPI_Isend(&data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &requests[1]);
        }
    } else if (rank == 1) {
        std::cout << "data: " << data << std::endl;
        for (int i=0; i < 9; i++){
          MPI_Irecv(&data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &requests[0]);
          std::cout << "Process 1 received data: " << data << std::endl;
        }
    }

    if (rank == 1){
      // do something else
      for ( int i = 0; i < 10000; i++ ){
      }
      std::cout << "Process 1 is done" << std::endl;
      MPI_Wait( &requests[0], MPI_STATUS_IGNORE );
      std::cout << "data: " << data << std::endl;
    }


    MPI_Finalize();
    return 0;
}
