#pragma DATASET_H

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <ctime>
#include <vector>
#include <cmath>
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
    void createClusters(int rank, MPI_Comm comm);
    void assignCluster(int ceneter, int *dataPoints, int *assign);
    void calcMean(int *center, int *dataPoints, int *assign);
};
