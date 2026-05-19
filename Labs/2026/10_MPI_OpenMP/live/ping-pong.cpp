#include <mpi.h>

#include <iostream>

int main (int argc, char **argv)
{
  MPI_Init (&argc, &argv);
  
  MPI_Comm mpi_comm = MPI_COMM_WORLD;
  
  int mpi_size;
  MPI_Comm_size (MPI_COMM_WORLD, &mpi_size);
  
  int mpi_rank;
  MPI_Comm_rank (mpi_comm, &mpi_rank);
  
//  std::cout << "Hello from processor " << mpi_rank
//            << " out of " << mpi_size << std::endl;
  
  if (mpi_size != 2)
  {
    if (mpi_rank == 0)
      std::cerr << "Ping pong must be played by 2 ranks\n";
    return 1;
  }
  
  
  int num_iterations = 10;
  int ping_pong_count = 0;
  int other_rank = !mpi_rank;
  
  MPI_Status status;
  
  for (int i = 0; i < num_iterations; ++i)
  {
    if (mpi_rank == ping_pong_count % 2)
    {
      ++ping_pong_count;
      
      //int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
      MPI_Send (&ping_pong_count, 1, MPI_INT, 
                other_rank,//my partner rank
                0,
                mpi_comm);
                
      std::cout << "Ping " << mpi_rank << " --> " <<
                   other_rank << " count " << ping_pong_count
                << std::endl;
    }
    else
    {
      //int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
      MPI_Recv (&ping_pong_count, 1, MPI_INT,
                other_rank,
                0,
                mpi_comm,
                &status);
           std::cout << "Pong " << mpi_rank << " <-- " <<
                   other_rank << " count " << ping_pong_count
                << std::endl;
    }
  
  }
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  MPI_Finalize ();
  
  return 0;
}