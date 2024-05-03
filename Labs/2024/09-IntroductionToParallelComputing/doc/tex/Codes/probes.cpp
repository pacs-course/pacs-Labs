#include <mpi.h>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
  
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  std::vector<int> v;

  if(rank == 0){
    v = {1, 2, 3, 4, 5};  
    // sending a vector of integers to processs 1, tag=0
    MPI_Send(v.data(),v.size(),MPI_INT,1,0,MPI_COMM_WORLD);
  }
  else if (rank == 1){
    int amount;
    MPI_Status status;
    MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &amount);
    v.resize(amount); //make sure v can hold the msg
    MPI_Recv(v.data(), 5, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  MPI_Finalize();
  return 0;

}
