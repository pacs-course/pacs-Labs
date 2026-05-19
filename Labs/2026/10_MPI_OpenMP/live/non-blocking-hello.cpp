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
  
  
  if (mpi_size != 2)
  {
    if (mpi_rank == 0)
      std::cerr << "Ping pong must be played by 2 ranks\n";
    return 1;
  }
  
  
  int number_to_receive = mpi_rank;
  int number_to_send = 2*mpi_rank;
  int other_rank = !mpi_rank;
  
  int tag_send = (mpi_rank ==0) ? 0 : 1;
  int tag_receive = (mpi_rank ==0) ? 1 : 0;
  
  MPI_Status status;
  /*
  int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
              MPI_Comm comm, MPI_Request *request)
  */
  MPI_Request request;
        MPI_Irecv (&number_to_receive, 1, MPI_INT,
                other_rank,
                tag_receive,
                mpi_comm,
                &request);
  
          MPI_Send (&number_to_send, 1, MPI_INT, 
                other_rank,
                tag_send,
                mpi_comm);
    



                

  
  
  MPI_Finalize ();
  
  return 0;
}