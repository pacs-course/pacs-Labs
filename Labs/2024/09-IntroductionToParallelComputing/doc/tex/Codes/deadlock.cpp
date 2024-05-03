#include <mpi.h>
#include <iostream> 

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int foo;                
    if (rank == 0)   {
        foo = 10;           
        int tag;            
        for (int dest=1; dest < size; dest++) {
            tag = dest;     
            if (dest != 2) 
            {    // CREATES A DEADLOCK!!!
                MPI_Send(&foo, 1, MPI_INT, dest, tag, MPI_COMM_WORLD); 
            }
        }
    }
    else  {
        int source = 0;      
        int tag = rank;      
        MPI_Recv(&foo, 1, MPI_INT, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    std::cout << "Hello from rank " << rank << " of " << size << "\n"
              << "Foo value is: " << foo <<  std::endl;


    MPI_Finalize();
}
