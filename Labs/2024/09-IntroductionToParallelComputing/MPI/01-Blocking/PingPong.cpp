#include <mpi.h>
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>

int main( int argc, char *argv[] )
{
    int value;
    MPI_Init( &argc, &argv );

    int size;
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    std::vector<MPI_Request> requests( size );

    int rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );

    if ( rank == 0 )
    {
        value = 42;
        for ( int i = 0; i < 9; i++ )
        {
            MPI_Isend( &value, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &requests[1] );
        }
    }
    else if ( rank == 1 )
    {

        for ( int i = 0; i < 9; i++ )
        {
          MPI_Irecv( &value, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,  &requests[0] );
          std::cout << "Process 1 received value " << value << " from process 0" << std::endl;
        }
    }

    if (rank == 1){
      // do something else
      for ( int i = 0; i < 10000; i++ ){
      }
      std::cout << "Process 1 is done" << std::endl;
      MPI_Wait( &requests[0], MPI_STATUS_IGNORE );
      std::cout << "value: " << value << std::endl;
    }

    MPI_Finalize();
    return 0;
}
