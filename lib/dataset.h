#pragma DATASET_H

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <ctime>
#include <vector>
#include <mpi.h>

struct Dataset
{
    int numberOfPoints;
    int numberOfClusters;
    int maxIteration;
    std::vector<int> pointList;
    std::vector<int> clusterList;
    std::string filename;

    Dataset(int numberOfPoints, int numberOfClusters, int maxIteration, std::string filename);

    void generateData();
    void readData(std::string filename);
    void showDataPoints();
    void showTheClusterData();
    void createClusters(int rank, MPI_Comm comm);
};
