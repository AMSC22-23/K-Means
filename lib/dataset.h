#pragma DATASET_H

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <ctime>
#include <vector>

struct Point
{
    int id;
    int value;
    Point(int pointId, int pointValue);
};

struct Cluster
{
    int id;
    int mean;
    std::vector<Point> data;

    Cluster(int id, std::vector<Point> listData);
    void calculateMean();
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
    void createClusters();
};
