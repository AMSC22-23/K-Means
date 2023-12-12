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

    // Define the custom MPI data type for the cluster struct
    MPI_Datatype mpi_cluster;
    int count = 2;                                // number of blocks in the struct
    int blocklengths[2] = {1, 25};                // number of elements in each block
    MPI_Aint displacements[2] = {0, sizeof(int)}; // offsets of each block in bytes
    MPI_Datatype types[2] = {MPI_INT, MPI_INT};
    MPI_Type_create_struct(count, blocklengths, displacements, types, &mpi_cluster); // create the new data type
    MPI_Type_commit(&mpi_cluster);                                                   // commit the new data type

    // Allocate and initialize the vector of integers to scatter on the root process
    std::vector<int> sendbuf;
    if (rank == root)
    {
        sendbuf.resize(100); // resize the vector to 100 elements
        for (int i = 0; i < 100; i++)
        {
            sendbuf[i] = i + 1; // fill the vector with 1, 2, ..., 100
        }
    }

    // Allocate and initialize the cluster struct to receive the data on each process
    Cluster recvbuf(rank);
    recvbuf.id = rank;
    recvbuf.data.resize(25);

    // Perform the scatter operation
    MPI_Scatter(sendbuf.data(), 25, MPI_INT,
                &recvbuf, 1, mpi_cluster,
                root, MPI_COMM_WORLD);

    // Print the cluster struct on each process
    std::cout << "Process " << rank << " received cluster: " << std::endl;
    std::cout << "id = " << recvbuf.id << std::endl;

    std::cout << std::endl;

    // Free the custom MPI data type
    MPI_Type_free(&mpi_cluster);

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
