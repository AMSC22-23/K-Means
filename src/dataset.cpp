#include "../lib/dataset.h"

Point::Point(int pointId, int pointValue)
{
    this->id = pointId;
    this->value = pointValue;
}

Dataset::Dataset(int numberOfPoints, int numberOfClusters, int maxIteraion, std::string filename)
{
    this->numberOfPoints = numberOfPoints;
    this->numberOfClusters = numberOfClusters;
    this->maxIteration = maxIteraion;
    this->filename = filename;
    this->pointList = {};
};

void Dataset::generateData()
{
    bool fileFlag = false;

    std::string path = "data/";
    if (std::filesystem::directory_entry(path).exists())
    {
        for (const auto &entry : std::filesystem::directory_iterator(path))
        {
            fileFlag = true;
            // loop through all the files in the folder
            std::string names = entry.path().filename().string(); // get the file name
            if (!(filename.rfind(filename, 0) == 0 && names.find(".csv") != std::string::npos))
            {
                std::ofstream dataFile;
                dataFile.open(path + this->filename + "-" + std::to_string(this->numberOfPoints) + "-" + std::to_string(this->numberOfClusters) + ".csv", std::ios_base::in);

                if (dataFile)
                {
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
                }
            }
        }

        if (fileFlag == false)
        {
            std::cout << fileFlag << std::endl;
            std::ofstream dataFile;
            dataFile.open(path + this->filename + "-" + std::to_string(this->numberOfPoints) + "-" + std::to_string(this->numberOfClusters) + ".csv");

            if (dataFile)
            {
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
            }
        }
    };
}

void Dataset::readData(std::string filename)
{
    std::string path = "data/";
    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        // loop through all the files in the folder
        std::string names = entry.path().filename().string(); // get the file name
        if (filename.rfind(filename, 0) == 0 && names.find(".csv") != std::string::npos)
        {
            // the file is a match
            // open and read the file
            std::ifstream myfile;      // create an input file stream object
            myfile.open(entry.path()); // open the file
            if (myfile.is_open())
            {
                // the file is open
                std::string line; // a string to store each line
                while (std::getline(myfile, line))
                {
                    // read each line until the end of the file
                    // process the line
                    int idNumber = 1;
                    std::stringstream ss(line);
                    while (std::getline(ss, line, ','))
                    {
                        Point point(idNumber, std::stod(line));
                        idNumber++;
                        this->pointList.push_back(point);
                    }
                }
                myfile.close(); // close the file
            }
            else
            {
                // the file could not be opened
                std::cerr << "Error: could not open the file." << std::endl;
            }
        }
    }
};

void Dataset::showDataPoints()
{
    for (int i = 0; i < this->pointList.size(); i++)
    {
        std::cout << this->pointList[i].id << ' ' << this->pointList[i].value << std::endl;
    }
}

Cluster::Cluster(int id, std::vector<Point> data)
{
    this->id = id;
    this->mean = 0;
    this->data = data;
}

void Dataset::createClusters()
{
    for (int i = 0; i < this->numberOfClusters; i++)
    {
        // create two local value to determine the range of the vector
        int b = 0, e = 0;
        if (i == 0)
        {
            b = b + (i * 20);
        }
        else
        {
            b = (b + (i * 20)) + 1;
        }
        e = e + 20 * (i + 1);

        std::vector<Point> temp(this->pointList.begin() + b, this->pointList.begin() + e);
        Cluster cl(i, temp);
        this->clusterList.push_back(cl);
    }
}

void Dataset::showTheClusterData()
{
    std::cout << "----- The Cluster -----" << std::endl;

    for (int i = 0; i < this->clusterList.size(); i++)
    {
        std::cout << this->clusterList[i].id << std::endl;
    }
}