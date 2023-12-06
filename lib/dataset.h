#pragma DATASET_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

struct Dataset
{
    int numberOfPoints;
    int numberOfClusters;
    int maxIteration;
    std::string filename;
    Dataset(int numberOfPoints, int numberOfClusters, int maxIteration, std::string filename);
    void generateData();
    void readData();
};
