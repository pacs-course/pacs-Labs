#include <mpi.h>
#include <omp.h>

#include <algorithm>

#include <iostream>
#include <vector>

/*
A  * x  = b
r c  c  = r

*/

#include <iostream>

int main (int argc, char **argv)
{
  MPI_Init (&argc, &argv);
  
  MPI_Comm mpi_comm = MPI_COMM_WORLD;
  
  int mpi_size;
  MPI_Comm_size (MPI_COMM_WORLD, &mpi_size);
  
  int mpi_rank;
  MPI_Comm_rank (mpi_comm, &mpi_rank);
  
  
  #pragma omp parallel master
    if (mpi_rank == 0)
      std::cout << "mpi_size " << mpi_size
                << "omp_size " << omp_get_num_threads ()
                << std::endl;
  
  int rows;
  int cols;
  
  std::vector <double> A, x, b;
  
  //displacements and send counts
  std::vector <int> send_counts (mpi_size), send_start_idx(mpi_size), 
                    recv_counts(mpi_size), recv_start_idx(mpi_size);
  
  
  if (mpi_rank = 0)
  {
    rows = 10;
    cols = 10;
    
    A.resize (rows*cols);
    x.resize (cols);
    b.resize (rows);
    
    for (int i = 0; i < rows; ++i)
    {
      for (int j = 0; j < cols; ++j)
      {
        A[i + j*cols] = i + j;
        std::cout << A[i + j*cols] << " ";
       }
     std::cout << "\n";
     }
    std::cout <<"\n\n"<< std::endl;
    for (int j = 0; j < cols; ++j)
    {
      x[j] = j*j*j;
      std::cout << x[j] << "\n";
    }
  }
  
  /*
  int MPI_Bcast( void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm )
  */
  MPI_Bcast (&rows, 1, MPI_INT, 0, mpi_comm);
  MPI_Bcast (&cols, 1, MPI_INT, 0, mpi_comm);
  
  x.resize (cols);
  MPI_Bcast (x.data (), cols, MPI_DOUBLE, 0, mpi_comm);
  
  /*
  A = [r1,
       r2,
      ...
       r10]
  size == 4
  
  rank 0: [r0; r1; r2]
  rank 1: [r3; r4; r5]
  rank 2: [r6; r7]
  rank 3: [r8; r9]
  
  */
  int count = rows / mpi_size;
  int remainder = rows - count * mpi_size;
  
  /*
  if my_rank < remainder
    i get 3 rows
  else 
    i get 2 rows
  */
  
  int start_idx = 0;
  for (int i = 0; i < mpi_size; ++i)
  {
    // we receive local_b, which is a local_r x 1
    recv_counts[i] = ( i < remainder ) ? (count + 1) : count;
    // we send local_A, which is a local_r x c
    send_counts[i] = recv_counts[i] * cols;
    
    recv_start_idx[i] = start_idx;
    send_start_idx[i] = start_idx * cols;
    
    start_idx += recv_counts[i];
  }
  
  int local_rows = (mpi_rank < remainder ) ? (count + 1) : count;
  
  std::vector <double> local_A (local_rows * cols);
  /*
  int MPI_Scatterv(const void *sendbuf, const int *sendcounts, const int *displs,
                 MPI_Datatype sendtype, void *recvbuf, int recvcount,
                 MPI_Datatype recvtype,
                 int root, MPI_Comm comm)
  */
  
  MPI_Scatterv (A.data (), send_counts.data (), send_start_idx.data (),
                MPI_DOUBLE, local_A.data (), local_rows * cols,
                MPI_DOUBLE, 0, mpi_comm);
                
  std::vector <double> local_b (local_rows, 0.);
  
  #pragma omp parallel for shared (local_b)
    for (int i = 0; i < local_rows; ++i)
      for (int j = 0; j < cols; ++j)
        local_b[i] = local_A[i + cols*j] * x[j];
  
  /*
  int MPI_Gatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                void *recvbuf, const int recvcounts[], const int displs[],
                MPI_Datatype recvtype, int root, MPI_Comm comm)
  */
  
  MPI_Gatherv (local_b.data (), local_rows, MPI_DOUBLE,
               b.data (), recv_counts.data (), recv_start_idx.data (),
               MPI_DOUBLE, 0, mpi_comm);
  
  if (mpi_rank == 0)
  {
    std:: cout << "\n\n";
    for (const auto & r: b)
      std::cout << r << "\n";
  }
  
  
  MPI_Finalize ();
  
  return 0;
}