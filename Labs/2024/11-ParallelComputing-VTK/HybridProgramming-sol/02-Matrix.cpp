#include <iostream>
#include <vector>
#include <mpi.h>
#include <omp.h>

#include <random>

static double c_start, c_diff;
#define tic() c_start = MPI_Wtime();
#define toc(x)                                       \
  {                                                  \
    c_diff = MPI_Wtime() - c_start;                  \
    std::cout << x << c_diff << " [s]" << std::endl; \
  }


int main( int argc, char** argv) {

  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // nrows, ncols
  int nrows, ncols;
  ncols = 10000;
  nrows = 10000;

  int local_nrows = (nrows % size > rank) ? nrows/size +1 : nrows/size;

  std::vector<int> send_counts(size, 0), send_start_idx(size, 0),
                   recv_counts(size, 0), recv_start_idx(size, 0);
  

  int start_idx = 0;
  for (int i = 0; i < size; ++i)
    {
      recv_counts[i] = (nrows % size > i) ? nrows / size + 1 : nrows / size;
      send_counts[i] = recv_counts[i] * ncols;

      recv_start_idx[i] = start_idx;
      send_start_idx[i] = start_idx * ncols;

      start_idx += recv_counts[i];
    }

  // define local matrix
  std::vector<double> A_local(local_nrows*ncols, 0.0);

  std::random_device                     engineMat;
  std::uniform_real_distribution<double> randMat(0, 1);
  for (auto &m : A_local){
    m = randMat(engineMat);
  }


  // define rhs
  std::vector<double> rhs(ncols, 0.0);

  std::vector<double> result;
  if ( rank == 0) {
    result.resize(nrows);

    std::random_device                     engine;
    std::uniform_real_distribution<double> rand(0, 1);
    for (auto &m : rhs){
      m = rand(engine);
    }
  }

  // bcast rhs
  MPI_Bcast(rhs.data(), ncols, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  
  // define result
  std::vector<double> result_local(local_nrows, 0.0);

  // perform matrix vector multiplication
// this will be omp
  tic();

#pragma omp parallel for shared(A_local, rhs, result_local)
    for (int i = 0; i < local_nrows; i++){
      for (int j = 0; j < ncols; j++){
        result_local[i] += A_local[i*ncols + j] * rhs[j];
      }
    }

  MPI_Barrier(MPI_COMM_WORLD);
  std::cout << std::endl;
  if ( rank == 0){
    toc(" matrix-vector products - Time elapsed on rank " +
          std::to_string(rank) + ": ");
  }

  MPI_Gatherv(result_local.data(), result_local.size(), MPI_DOUBLE, 
                    result.data(),  recv_counts.data(), 
            recv_start_idx.data(), MPI_DOUBLE, 0, MPI_COMM_WORLD);

  
  MPI_Finalize();
  return 0;
}


