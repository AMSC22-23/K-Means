
#include <iostream>
#include <mpi.h>

#include "./lib/dataset.h"

#define NUM_THREAD 4

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    Dataset data(100, 5, 2, "test");
    data.generateData();
    data.readData("test");
    //  data.showDataPoints();
    data.createClusters();
    data.showTheClusterData();
}