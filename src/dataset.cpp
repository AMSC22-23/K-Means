#include "../lib/dataset.h"

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
                    std::stringstream ss(line);
                    while (std::getline(ss, line, ','))
                    {
                        this->pointList.push_back(std::stod(line));
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

void Dataset::createClusters(int rank, MPI_Comm comm)
{
    const int number_of_processors = 4;
    const int number_of_iterations = 20;
    // Define the root process
    int root = 0;

    int *nodeArray = new int[4];

    int *sendBuffer;
    // send operation
    if (rank == 0)
    {

        sendBuffer = new int[100];

        for (int i = 0; i < this->numberOfPoints; i++)
        {
            sendBuffer[i] = this->pointList[i];
        }

        // randomly select the center points for each cluster
        for (int i = 0; i < this->numberOfClusters; i++)
        {
            int random = std::rand() % 24;
            nodeArray[i] = sendBuffer[random];
            cluster[sendBuffer[random]];
        }

        // brodcast the number of clusters to all the nodes
        MPI_Bcast(&this->numberOfClusters, 1, MPI_INT, root, comm);
        // brodcast the number of data points to all the nodes
        MPI_Bcast(&this->numberOfPoints, 1, MPI_INT, root, comm);
    }
    else
    {
        MPI_Bcast(&this->numberOfClusters, 1, MPI_INT, root, comm);

        MPI_Bcast(&this->numberOfPoints, 1, MPI_INT, root, comm);
    }

    MPI_Bcast(nodeArray, 4, MPI_INT, root, comm);

    // receive buffer
    int *recvBuffer = new int[25];

    MPI_Scatter((void *)sendBuffer, this->numberOfPoints / number_of_processors, MPI_INT, (void *)recvBuffer, this->numberOfPoints / number_of_processors, MPI_INT, root, comm);

    // TODO: do this until reach the max iteration
    std::map<int, std::vector<int>> assignCluster = this->initAssignCluster(nodeArray[rank], recvBuffer, cluster);

    std::vector<int> gatherData;
    gatherData.resize(100);

    std::map<int, std::vector<int>> gatherCluster;

    const int size = 1;

    int buffer_size = 0;
    char *buffer = nullptr;
    char *gather_buffer = nullptr;

    buffer_size += sizeof(int);
    for (const auto &p : assignCluster)
    {
        // buffer_size += sizeof(int);
        buffer_size += p.second.size() * sizeof(int);
    }

    buffer = new char[buffer_size];

    // pack the map into the buffer
    int position = 0;
    int num_keys = assignCluster.size();
    MPI_Pack(&num_keys, 1, MPI_INT, buffer, buffer_size, &position, comm);
    for (const auto &p : assignCluster)
    {
        int key = p.first;
        MPI_Pack(&key, 1, MPI_INT, buffer, buffer_size, &position, comm);
        int value_size = p.second.size();
        MPI_Pack(&value_size, 1, MPI_INT, buffer, buffer_size, &position, comm);
        MPI_Pack(p.second.data(), value_size, MPI_INT, buffer, buffer_size, &position, comm);
    }

    // gather the size of the buffer from all processes
    int *gather_buffer_size = new int[size];
    MPI_Gather(&buffer_size, 1, MPI_INT, gather_buffer_size, 1, MPI_INT, 0, comm);

    // allocate memory for the gather buffer on the root process
    int total_buffer_size = 0;
    for (int i = 0; i < size; i++)
    {
        total_buffer_size += gather_buffer_size[i];
    }
    gather_buffer = new char[total_buffer_size];

    // gather the buffer from all processes
    int *displs = new int[size];
    displs[0] = 0;
    for (int i = 1; i < size; i++)
    {
        displs[i] = displs[i - 1] + gather_buffer_size[i - 1];
    }
    MPI_Gatherv(buffer, buffer_size, MPI_PACKED, gather_buffer, gather_buffer_size, displs, MPI_PACKED, 0, comm);

    // unpack the buffer into the map on the root process
    if (rank == 0)
    {
        int position1 = 0;
        for (int i = 0; i < size; i++)
        {
            int num_keys1;
            MPI_Unpack(gather_buffer, gather_buffer_size[i], &position1, &num_keys1, 1, MPI_INT, comm);
            for (int j = 0; j < num_keys1; j++)
            {
                int key;
                MPI_Unpack(gather_buffer, gather_buffer_size[i], &position1, &key, 1, MPI_INT, comm);
                int value_size;
                MPI_Unpack(gather_buffer, gather_buffer_size[i], &position1, &value_size, 1, MPI_INT, comm);
                int *value = new int[value_size];
                MPI_Unpack(gather_buffer, gather_buffer_size[i], &position1, value, value_size, MPI_INT, comm);
                gatherCluster[key] = std::vector<int>(value, value + value_size);
                delete[] value;
            }
        }
        // print the map on the root process
        for (const auto &p : gatherCluster)
        {

            for (int x : p.second)
            {
                std::cout << "Process " << rank << ": "
                          << p.first << " " << x << "  " << std::endl;
            }
            std::cout << std::endl;
        }
    }

    delete[] nodeArray;
    delete[] sendBuffer;
    delete[] recvBuffer;
}

std::map<int, std::vector<int>> Dataset::initAssignCluster(int center, int *dataPoints, std::map<int, std::vector<int>> cluster)
{

    for (int i = 0; i < 25; i++)
    {
        this->cluster[center].emplace_back(dataPoints[i]);
    }
    return this->cluster;
}

void Dataset::reAssignCluster(int *center, int *dataPoints, std::map<int, std::vector<int>> cluster)
{
    int x = 0;
    int tmp = 0;
    int minDist = 100000000;
    for (int i = 0; i < (this->numberOfPoints / 4); i++)

        for (int j = 0; j < this->numberOfClusters; j++)
        {
            x = abs(center[j] - dataPoints[i]);
            tmp = std::sqrt(std::pow(x, 2));
            int k_index = 0, v_index = 0;
            int counterIndex = 0,
                counterValue = 0;

            for (auto const &t : this->cluster)
            {

                for (auto const &l : t.second)
                {
                    if (dataPoints[i] == l)
                    {
                        // store the location of previous cluster
                        k_index = counterIndex;
                        v_index = counterValue;
                    }
                    counterValue += 1;
                }
                counterIndex += 1;
            }

            if (tmp < minDist)
            {
                minDist = tmp;
                // this->cluster[k_index].erase(this->cluster[k_index].begin() + v_index);
                // std::cout << center[j] << std::endl;
                this->cluster[center[j]].push_back(dataPoints[i]);
            }
        }
}

void Dataset::calcMean(int *ceneter, int *dataPoints, int *assign)
{
    for (int j = 0; j < this->numberOfClusters; j++)
    {
        // std::cout << "The center is: " << ceneter[j] << " and data values are : " << std::endl;
        for (int i = 0; i < this->numberOfPoints / 4; i++)
        {
            //   std::cout << dataPoints[i] << std::endl;
        }
    }
}

//    MPI_Gather((void *)recvBuffer, 25, MPI_INT, (void *)gatherData.data(), 25, MPI_INT, root, comm);

// MPI_Bcast(gatherData.data(), 100, MPI_INT, root, comm);

// for (int j = 0; j < this->cluster[nodeArray[rank]].size(); j++)
//{
//  std::cout << nodeArray[rank] << " " << this->cluster[nodeArray[rank]][j] << std::endl;
//}

//    MPI_Gather((void *)assignRecv, (this->numberOfPoints / number_of_processors), MPI_INT, (void *)clusterAssignment, (this->numberOfPoints / number_of_processors), MPI_INT, root, comm);

//    if (rank == 0)
//        for (int i = 0; i < 25; i++)
//        {
//            std::cout << "Things i'm receiving is: " << clusterAssignment[i] << std::endl;
//        }
//    {
//        this->calcMean(nodeArray, recvBuffer, clusterAssignment);
//    }