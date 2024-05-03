#include <iostream>
#include <mpi.h>


double f(double x)
{
    return x*x + 3*x + 1;
}


int main( int argc, char* argv[])
{
    // we initialize to give default values
    double a = 0, b = 1, sum = 0;
    int n = 100;   // number of intervals

    int rank, size;

    MPI_Init( &argc , &argv );
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0)
    {
        std::cout << "Insert extrema a: " << std::endl;
        std::cin >> a;
        std::cout << "Insert extrema b: " << std::endl;
        std::cin >> b;
        std::cout << "Insert number of intervals: " << std::endl;
        std::cin >> n;      // to simplify, n * np
    }

    MPI_Bcast(&a,1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&b,1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n,1, MPI_INT,    0, MPI_COMM_WORLD);

    double h;
    h = (b - a) / (n*size);    // discretization size

    double ai, bi; // extremes of composite integration

    for (int i= rank*n; i < (rank+1)*n; i++)
    {
        ai = a + i*h; // ai
        bi = a + (i+1)*h; // bi
        sum = sum +  ( f( ai ) + f( bi ));
    }

    sum = sum * h/2;            // quadrature output

    if (rank == 0)              // MPI is weird
    {
        MPI_Reduce(MPI_IN_PLACE, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Reduce(&sum, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    }


    if (rank == 0) 
    {
    std::cout << "Integral: " << sum   << std::endl;
    std::cout << "Exact:    " << (1.0/3*b*b*b + 1.5*b*b + b) - 
                                 (1.0/3*a*a*a + 1.5*a*a + a) << std::endl;
    }

    MPI_Finalize();
    
}
