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

    // Define a custom MPI datatype for Point
    //    MPI_Datatype MPI_Point;
    //    int blocklengths[2] = {1, 1};
    //    MPI_Aint displacements[2] = {offsetof(Point, id), offsetof(Point, value)};
    //    MPI_Datatype types[2] = {MPI_INT, MPI_INT};
    //    MPI_Type_create_struct(2, blocklengths, displacements, types, &MPI_Point);
    //    MPI_Type_commit(&MPI_Point);
    //
    //    // Define a custom MPI datatype for cluster
    //    MPI_Datatype MPI_cluster;
    //    int blocklengths2[2] = {1, 1};
    //    MPI_Aint displacements2[2] = {offsetof(Cluster, id), offsetof(Cluster, data)};
    //    MPI_Datatype types2[2] = {MPI_INT, MPI_Point};
    //    MPI_Type_create_struct(2, blocklengths2, displacements2, types2, &MPI_cluster);
    //    MPI_Type_commit(&MPI_cluster);
    //
    int num_procs = 4;

    // Create an array of cluster on the root process
    // Create an array of Cluster on the root process
    Cluster *array = NULL;
    if (rank == 0)
    {
        // Change the size of the array to 100
        array = new Cluster[100];
        // Fill the array with some data using constructors
        for (int i = 0; i < 100; i++)
        {
            std::vector<Point> points;
            for (int j = 0; j < i + 1; j++)
            {
                points.push_back(Point(j, i * j)); // Use Point constructor
            }
            array[i] = Cluster(i, points); // Use Cluster constructor
        }
    }

    // Create a buffer of Cluster on each process
    // Change the size of the buffer to 100 / num_procs
    Cluster *buffer = new Cluster[100 / num_procs];

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

    // Create an array of send_counts and displacements on the root process
    int *send_counts = NULL;
    int *displs = NULL;
    if (rank == 0)
    {
        // Allocate the array of send_counts and displacements
        send_counts = new int[num_procs];
        displs = new int[num_procs];
        // Get the size of the MPI_PACKED type in bytes
        int packed_size = 0;
        MPI_Type_size(MPI_PACKED, &packed_size);
        // Fill the array of send_counts and displacements
        int offset = 0;
        for (int i = 0; i < num_procs; i++)
        {
            // Calculate the send_count for each process based on the size of the Cluster
            send_counts[i] = (2 + array[i].data.size() * 2) * packed_size;
            // Assign the displacement for each process based on the offset
            displs[i] = offset;
            // Update the offset for the next process
            offset += send_counts[i];
        }
    }

    // Create a variable for recv_count on each process
    int recv_count = 0;

    // Scatter the send_counts from the root to all processes
    MPI_Scatter(send_counts, 1, MPI_INT, &recv_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Create a receive buffer on each process
    int *recvbuf = NULL;
    // Allocate the receive buffer based on recv_count
    recvbuf = new int[recv_count / sizeof(int)];

    // Scatter the send buffer from the root to all processes
    // Use send_counts and displs as parameters for MPI_Scatter
    MPI_Scatterv(sendbuf, send_counts, displs, MPI_PACKED, recvbuf, recv_count, MPI_PACKED, 0, MPI_COMM_WORLD);

    // Unpack the receive buffer into the buffer of Cluster
    int position = 0;
    for (int i = 0; i < 100 / num_procs; i++)
    {
        MPI_Unpack(recvbuf, recv_count, &position, &buffer[i].id, 1, MPI_INT, MPI_COMM_WORLD);
        int vector_size = 0;
        MPI_Unpack(recvbuf, recv_count, &position, &vector_size, 1, MPI_INT, MPI_COMM_WORLD);
        buffer[i].data.resize(vector_size);
        for (int j = 0; j < vector_size; j++)
        {
            MPI_Unpack(recvbuf, recv_count, &position, &buffer[i].data[j].id, 1, MPI_INT, MPI_COMM_WORLD);
            MPI_Unpack(recvbuf, recv_count, &position, &buffer[i].data[j].value, 1, MPI_INT, MPI_COMM_WORLD);
        }
    }

    // Print the received data on each process
    printf("Process %d received %d clusters:\n", rank, 100 / num_procs);
    for (int i = 0; i < 100 / num_procs; i++)
    {
        printf("Cluster with id %d and data:\n", buffer[i].id);
        for (int j = 0; j < buffer[i].data.size(); j++)
        {
            printf("Point with id %d and value %d\n", buffer[i].data[j].id, buffer[i].data[j].value);
        }
        printf("\n");
    }

    // Free the memory
    delete[] buffer;
    delete[] recvbuf;
    if (rank == 0)
    {
        delete[] array;
        delete[] sendbuf;
        delete[] send_counts;
        delete[] displs;
    }

    // Create an array of cluster on the root process
    // Cluster *array = NULL;
    // if (rank == 0)
    // {
    //     array = new Cluster[4];
    //     // Fill the array with some data using constructors
    //     for (int i = 0; i < 4; i++)
    //     {
    //         std::vector<Point> temp(this->pointList.begin() + 0, this->pointList.begin() + 25);
    //         array[i] = Cluster(i, temp);
    //     }
    // }

    // // Create a buffer of cluster on each process

    // Cluster *buffer = new Cluster[1];
    // // buffer->data.resize(25);

    // MPI_Scatter((void *)array, 25, MPI_cluster, (void *)buffer, 25, MPI_cluster, 0, MPI_COMM_WORLD);

    // // Print the received data on each process
    // printf("Process %d received cluster with id %d and data\n", rank, buffer->id);
    // for (int i = 0; i < 5; i++)
    // {
    //     printf("Point with id %d and value %d\n", buffer->data[i].id, buffer->data[i].value);
    // }

    // // Free the memory
    // delete[] buffer;
    // if (rank == 0)
    // {
    //     delete[] array;
    // }
}

void Dataset::showTheClusterData()
{
    std::cout << "----- The Cluster -----" << std::endl;

    for (int i = 0; i < this->clusterList.size(); i++)
    {
        std::cout << this->clusterList[i].id << std::endl;
    }
}
