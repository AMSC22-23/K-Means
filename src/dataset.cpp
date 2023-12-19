//@note: you should include files without specifing here the path
//       you should instead add the proper flags to the compiler to indicate the
//       include path
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

void Dataset::createClusters(int rank, MPI_Comm comm)
{

    // Define the root process
    int root = 0;

    // Create an array of Cluster on the root process
    //@note: should prefer managed memory (std::vector to this)
    //@note: `NULL` is deprecated in C++, use `nullptr` instead
    // similar comments hold true for several points in the code below
    Cluster *array = NULL;
    if (rank == 0)
    {
        //@note: why 100? if it is the number of clusters it should at least be a parameter
        array = new Cluster[100];
        // Fill the array with some data using constructors
        for (int i = 0; i < 100; i++)
        {
            std::vector<Point> points;
            for (int j = 0; j < i + 1; j++)
            {
                //@note: here could have been a nice application of `emplace_back`
                points.push_back(Point(j, i * j)); // Use Point constructor
            }
            array[i] = Cluster(i, points); // Use Cluster constructor
        }
    }

    // Create a buffer of Cluster on each process
    Cluster *buffer = new Cluster[100];

    // Create a send buffer on the root process
    int *sendbuf = NULL;
    int sendbuf_size = 0;
    if (rank == 0)
    {
        // Calculate the size of the send buffer
        for (int i = 0; i < 100; i++)
        {
            sendbuf_size += 2 * sizeof(int);                        // For id and vector size
            sendbuf_size += array[i].data.size() * 2 * sizeof(int); // For vector data
        }
        // Allocate the send buffer
        sendbuf = new int[sendbuf_size / sizeof(int)];
        // Pack the array of Cluster into the send buffer
        int position = 0;
        for (int i = 0; i < 100; i++)
        {
            MPI_Pack(&array[i].id, 1, MPI_INT, sendbuf, sendbuf_size, &position, MPI_COMM_WORLD);
            int vector_size = array[i].data.size();
            MPI_Pack(&vector_size, 1, MPI_INT, sendbuf, sendbuf_size, &position, MPI_COMM_WORLD);
            for (int j = 0; j < vector_size; j++)
            {
                MPI_Pack(&array[i].data[j].id, 1, MPI_INT, sendbuf, sendbuf_size, &position, MPI_COMM_WORLD);
                MPI_Pack(&array[i].data[j].value, 1, MPI_INT, sendbuf, sendbuf_size, &position, MPI_COMM_WORLD);
            }
        }
    }

    // Create a receive buffer on each process
    int *recvbuf = NULL;
    int recvbuf_size = 0;
    // Calculate the size of the receive buffer
    recvbuf_size += 2 * sizeof(int);              // For id and vector size
    recvbuf_size += (rank + 1) * 2 * sizeof(int); // For vector data
    // Allocate the receive buffer
    recvbuf = new int[recvbuf_size / sizeof(int)];

    // Scatter the send buffer from the root to all processes
    MPI_Scatter(sendbuf, recvbuf_size, MPI_PACKED, recvbuf, recvbuf_size, MPI_PACKED, 0, MPI_COMM_WORLD);

    // Unpack the receive buffer into the buffer of Cluster
    int position = 0;
    MPI_Unpack(recvbuf, recvbuf_size, &position, &buffer->id, 1, MPI_INT, MPI_COMM_WORLD);
    int vector_size = 0;
    MPI_Unpack(recvbuf, recvbuf_size, &position, &vector_size, 1, MPI_INT, MPI_COMM_WORLD);
    buffer->data.resize(vector_size);

    for (int i = 0; i < vector_size; i++)
    {
        MPI_Unpack(recvbuf, recvbuf_size, &position, &buffer->data[i].id, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(recvbuf, recvbuf_size, &position, &buffer->data[i].value, 1, MPI_INT, MPI_COMM_WORLD);
    }

    //@note: you are moving data around but how are you building clusters?

    // Print the received data on each process
    //@note: I know `std::cout` is a bit clunky but it is more idiomatic and should be preferred
    //       However, since C++23 we will have std::format which is better!
    printf("Process %d received %d clusters:\n", rank, 100 / 4);

    printf("Cluster with id %d and data:\n", buffer->id);
    for (int j = 0; j < buffer->data.size(); j++)
    {
        printf("Point with id %d and value %d\n", buffer->data[j].id, buffer->data[j].value);
    }
    printf("\n");

    // Free the memory
    delete[] buffer;
    delete[] recvbuf;
    if (rank == 0)
    {
        delete[] array;
        delete[] sendbuf;
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
