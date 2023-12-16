#include "../lib/dataset.h"
#include <mpi.h>

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

Cluster::Cluster(int id)
{
    this->id = id;
    this->mean = 0;
}

Cluster::Cluster(int id, std::vector<Point> data)
{
    this->id = id;
    this->data = data;
}

void Dataset::createClusters()
{
    // Get the rank of the current process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Get the total number of processes
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Check that the number of processes is 4
    if (size != 4)
    {
        std::cerr << "This program requires 4 processes" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    // Define the root process
    int root = 0;

    // Define a custom MPI datatype for Point
    MPI_Datatype MPI_Point;
    int blocklengths[2] = {1, 1};
    MPI_Aint displacements[2] = {offsetof(Point, id), offsetof(Point, value)};
    MPI_Datatype types[2] = {MPI_INT, MPI_INT};
    MPI_Type_create_struct(2, blocklengths, displacements, types, &MPI_Point);
    MPI_Type_commit(&MPI_Point);

    // Define a custom MPI datatype for cluster
    MPI_Datatype MPI_cluster;
    int blocklengths2[2] = {1, 1};
    MPI_Aint displacements2[2] = {offsetof(Cluster, id), offsetof(Cluster, data)};
    MPI_Datatype types2[2] = {MPI_INT, MPI_Point};
    MPI_Type_create_struct(2, blocklengths2, displacements2, types2, &MPI_cluster);
    MPI_Type_commit(&MPI_cluster);

    // Create an array of cluster on the root process
    Cluster *array = NULL;
    if (rank == 0)
    {
        array = new Cluster[4];

        for (int i = 0; i < 4; i++)
        {
            // create two local value to determine the range of the vector
            int b = 0, e = 0;
            if (i == 0)
            {
                b = b + (i * 25);
            }
            else
            {
                b = (b + (i * 25)) + 1;
            }
            e = e + 25 * (i + 1);
            std::vector<Point> temp(this->pointList.begin() + b, this->pointList.begin() + e);
            Cluster cl(i, temp);
            array[i] = cl;
        }
    }

    // Create a buffer of cluster on each process
    Cluster *buffer = new Cluster[1];

    // Scatter the array of cluster from the root to all processes
    MPI_Scatter(array, 25, MPI_cluster, buffer, 25, MPI_cluster, 0, MPI_COMM_WORLD);

    // Print the received data on each process
    printf("Process %d received cluster with id %d and data:\n", rank, buffer->id);
    for (int i = 0; i < buffer->data.size(); i++)
    {
        printf("Point with id %d and value %d\n", buffer->data[i].id, buffer->data[i].value);
    }
    printf("\n");

    // Free the memory
    delete[] buffer;
    if (rank == 0)
    {
        delete[] array;
    }

    MPI_Finalize();
}

void Dataset::showTheClusterData()
{
    std::cout << "----- The Cluster -----" << std::endl;

    for (int i = 0; i < this->clusterList.size(); i++)
    {
        std::cout << this->clusterList[i].id << std::endl;
    }
}
