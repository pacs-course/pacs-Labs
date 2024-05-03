#include <mpi.h>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<int> counts_send, displacements;

    std::vector<double> v;
    for (int i = 0; i < 1000; ++i) {
        v.push_back(0.1*i);
    }
    int n = v.size();

		if(rank ==0) {                      // root has to prepare some data
			counts_send.resize(size);
			displacements.resize(size,0); // init. by zero
                                        
			unsigned int chunk = n/size;      // integer division
			unsigned int rest  = n%size;      // the rest of the division
                                               
			for (int i=0; i<size; ++i){
				counts_send[i]= i < rest ? chunk+1 : chunk;
				if(i>0)
          displacements[i] = displacements[i-1] + counts_send[i-1];
			}
		}

		int local_size; // each process gets local size with normal scatter
		MPI_Scatter(counts_send.data(),1,MPI_INT, &local_size,1,MPI_INT,0,MPI_COMM_WORLD);

		//each process gets localsize its chunk of v
		std::vector<double> local_v(local_size);
		MPI_Scatterv(v.data(),counts_send.data(),displacements.data(),MPI_DOUBLE, 
                local_v.data(),local_size, MPI_DOUBLE,0,MPI_COMM_WORLD);

    std::cout << "Rank " << rank << " received " << local_v[local_size-1] 
                                 << "\nSize:" << local_size <<  std::endl;

    MPI_Finalize();
    return 0;
}
