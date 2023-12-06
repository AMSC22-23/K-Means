#include "../lib/dataset.h"

Dataset::Dataset(int numberOfPoints, int numberOfClusters, int maxIteraion, std::string filename)
{
    int numberOfPoints = this->numberOfPoints;
    int numberOfClusters = this->numberOfClusters;
    int maxIteration = this->maxIteration;
    std::string filename = this->filename;
};

void Dataset::generateData()
{
    std::ofstream dataFile;
    dataFile.open("data/" + this->filename + "-" + std::to_string(this->numberOfPoints) + "-" + std::to_string(this->numberOfClusters) + ".csv");

    // set a seed
    std::srand(std::time(NULL));
    for (int p = 0; p < this->numberOfPoints; p++)
    {
        dataFile << std::rand()
                 << ",";
    }
    // end of file
    dataFile << "\n";

    dataFile.close();
};
void Dataset::readData(){};
