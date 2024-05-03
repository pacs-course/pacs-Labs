#include <mpi.h>
#include <array>
#include <iostream>
#include <tuple>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    double a,b,c;
    std::array<double,2> v;

    if ( rank == 0){
      a=0.5, b=0.3, c=0.7;
      v[0]=0.1, v[1]=0.2;
    }

		if (rank==0)
		{
			// computes a,b,c and v
			std::tuple<double,double,double,std::array<double,2>> 
                        pack={a,b,c,v};
			// size in bytes
			int pack_siz=sizeof(pack);
			MPI_Send(&pack,pack_siz,MPI_BYTE,1,0,MPI_COMM_WORLD);
		} else
		{
			std::tuple<double,double,double,std::array<double,2>> pack;
			int pack_siz=sizeof(pack);
			MPI_Recv(&pack,pack_siz,MPI_BYTE,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			std::tie(a,b,c,v)=pack; // unpack the returned tuple
		}
    std::cout << "Rank: " << rank << " a: " << a << 
                 " b: " << b << " c: " << c << " v: " << v[0] << " " << v[1] << std::endl;

    MPI_Finalize();
    return 0;
}
