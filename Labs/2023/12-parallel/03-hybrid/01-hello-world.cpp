#include <iostream>
#include <mpi.h>
#include <omp.h>

int main(int argc, char *argv[]) {
    int rank, mpi_parallel_lvl_provided;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &mpi_parallel_lvl_provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

#pragma omp parallel
    {
      int omp_rank = omp_get_thread_num();
      std::cout << "I'm thread " << omp_rank << " in process " << rank << " \n";
    }

    if (rank == 0) {
        std::cout << "Provided thread support level: " << mpi_parallel_lvl_provided << " \n";
        std::cout << "  " << MPI_THREAD_SINGLE     << "  - MPI_THREAD_SINGLE\n";
        std::cout << "  " << MPI_THREAD_FUNNELED   << "  - MPI_THREAD_FUNNELED\n";
        std::cout << "  " << MPI_THREAD_SERIALIZED << "  - MPI_THREAD_SERIALIZED\n";
        std::cout << "  " << MPI_THREAD_MULTIPLE   << "  - MPI_THREAD_MULTIPLE\n";
    }
    MPI_Finalize();
    return 0;
}