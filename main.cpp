
#include <iostream>
#include <mpi.h>

#include "./lib/dataset.h"

int main(int argc, char *argv[])
{
    Dataset data(100, 5, 2, "test");
    data.generateData();
    data.readData("test");
    data.showDataPoints();
}