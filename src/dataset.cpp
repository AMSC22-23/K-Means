#include "../lib/dataset.h"

Dataset::Dataset(int numberOfPoints, int numberOfClusters, int maxIteraion, std::string filename)
{
    this->numberOfPoints = numberOfPoints;
    this->numberOfClusters = numberOfClusters;
    this->maxIteration = maxIteraion;
    this->filename = filename;
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
void Dataset::readData(std::string filename)
{
    std::string path = "data/";
    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        // loop through all the files in the folder
        std::string names = entry.path().filename().string(); // get the file name
        std::cout << "\ntest: " << names << std::endl;
        if (filename.rfind(filename, 0) == 0 && names.find(".csv") != std::string::npos)
        {
        }
    }
};
