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

struct Dataset
{
    int numberOfPoints;
    int numberOfClusters;
    int maxIteration;
    std::vector<Point> pointList;
    std::string filename;
    Dataset(int numberOfPoints, int numberOfClusters, int maxIteration, std::string filename);
    void generateData();
    void readData(std::string filename);
    void showDataPoints();
};
