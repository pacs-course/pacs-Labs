#include <mpi.h>
#include <iostream> 

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

int foo;                // foo is declared on all ranks
if (rank == 0)   {
    foo = 10;           // foo value on rank zero
    int tag;            // define a tag variable
    for (int dest=1; dest < size; dest++) {
        tag = dest;     // tag is given by the destination of the msg
        MPI_Send(&foo, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
    }
    foo = 20;           // foo new value on rank zero
    for (int dest=1; dest < size; dest++) {
        tag = dest + 1000;  // shift tag
        MPI_Send(&foo, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
    }
}
else  {
    int source = 0;      // the rank communicating is the rank zero
    int tag = rank;      // the tag is given by the rank

    MPI_Recv(&foo, 1, MPI_INT, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    std::cout << "Hello from rank " << rank << " of " << size << "\n"
              << "Foo value is: " << foo <<  std::endl;

    MPI_Recv(&foo, 1, MPI_INT, source, tag+1000, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    std::cout << "Hello from rank " << rank << " of " << size << "\n"
              << "Foo value is: " << foo <<  std::endl;
}


    MPI_Finalize();
}
