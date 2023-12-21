
#include <iostream>
#include <mpi.h>

#include "./lib/dataset.h"

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    // Get the rank of the current process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Get the total number of processes
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Check that the number of processes is 4
    if (size < 0)
    {
        std::cerr << "This program requires 4 processes" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    Dataset data(100, 4, 2, "test");
    data.generateData();
    data.readData("test");
    //  data.showDataPoints();
    data.createClusters(rank, MPI_COMM_WORLD);
    // data.showTheClusterData();

    MPI_Finalize();
}