#include <mpi.h>
#include <stdio.h>

int main( int argc, char *argv[])
{
    int  rank, size;
    char message[100];
    int  dest, source, tag=0;

    MPI_Init(&argc,&argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank != 0)
    {
        sprintf(message,"Greetings from process %d !\n",rank);
        dest = 0;
        tag = 0;
        MPI_Send(message, sizeof(message),MPI_CHAR, dest, tag, MPI_COMM_WORLD);
    } 
    else 
    {
        for (source = 1; source < size; source++)
        {
            MPI_Recv(message, 100, MPI_CHAR, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("%s",message);
        }
    }

    MPI_Finalize();
    return 0;
}
