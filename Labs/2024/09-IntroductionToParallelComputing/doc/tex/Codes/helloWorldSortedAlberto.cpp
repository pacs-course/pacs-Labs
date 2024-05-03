#include <mpi.h>
#include <stdio.h>


int main( int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int size, rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char message[100]; // message is a predefined array of chars
    if (rank != 0){
        sprintf(message, "Hello from rank %i of %i\n", rank, size); // message
        MPI_Send(message, 100, MPI_CHAR, 0, 0, MPI_COMM_WORLD);     // send  message
    }
    else {
        sprintf(message, "Hello from rank %i of %i\n", rank, size);
        printf("%s", message);

        for (int source=1; source < size; source++)
        {
            MPI_Recv(message, 100, MPI_CHAR, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("%s", message); // print message
        }
    }

    MPI_Finalize();
    return 0;
}
