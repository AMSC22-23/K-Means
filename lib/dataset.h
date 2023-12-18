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

struct Cluster
{
    int id;
    int mean;
    std::vector<Point> data;

    Cluster(int id);
    Cluster(int id, std::vector<Point> data);
    Cluster() : id(1), data(1, Point(0, 0)) {}
    // void calculateMean();
};

struct Dataset
{
    int numberOfPoints;
    int numberOfClusters;
    int maxIteration;
    std::vector<Point> pointList;
    std::vector<Cluster> clusterList;
    std::string filename;

    Dataset(int numberOfPoints, int numberOfClusters, int maxIteration, std::string filename);

    void generateData();
    void readData(std::string filename);
    void showDataPoints();
    void showTheClusterData();
    void createClusters(int rank, MPI_Comm comm);
};
