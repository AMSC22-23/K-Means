#pragma DATASET_H

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <ctime>
#include <vector>
#include <cmath>
#include <map>
#include <mpi.h>

struct Dataset
{
    int numberOfPoints;
    int numberOfClusters;
    int maxIteration;
    std::map<int, std::vector<int>> cluster;
    std::vector<int> pointList;
    std::vector<int> clusterList;
    std::string filename;

    Dataset(int numberOfPoints, int numberOfClusters, int maxIteration, std::string filename);

    void generateData();
    void readData(std::string filename);
    void createClusters(int rank, MPI_Comm comm);
    void initAssignCluster(int *ceneter, int *dataPoints, std::map<int, std::vector<int>> cluster);

    void reAssignCluster(int *ceneter, int *dataPoints, std::map<int, std::vector<int>> cluster);

    void calcMean(int *center, int *dataPoints, int *assign);
};
