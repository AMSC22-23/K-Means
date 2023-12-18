#pragma DATASET_H

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <ctime>
#include <vector>
#include <mpi.h>

struct Point
{
    int id;
    int value;
    Point() : id(0), value(0){};
    Point(int pointId, int pointValue);
};

struct Dataset
{
    int numberOfPoints;
    int numberOfClusters;
    int maxIteration;
    std::vector<int> pointList;
    std::string filename;

    Dataset(int numberOfPoints, int numberOfClusters, int maxIteration, std::string filename);

    void generateData();
    void readData(std::string filename);

    void createClusters(int rank, MPI_Comm comm);
};
