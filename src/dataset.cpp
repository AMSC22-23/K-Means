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
    // assignment receiver
    int *assignRecv = new int[100];

    MPI_Scatter((void *)sendBuffer, this->numberOfPoints / number_of_processors, MPI_INT, (void *)recvBuffer, this->numberOfPoints / number_of_processors, MPI_INT, root, comm);

    // TODO: do this until reach the max iteration
    //    MPI_Scatter((void *)clusterAssignment, this->numberOfPoints / number_of_processors, MPI_INT, (void *)assignRecv, this->numberOfPoints / number_of_processors, MPI_INT, root, comm);

    if (rank == 0)
    {
        this->initAssignCluster(nodeArray, recvBuffer, cluster);
        int buffer_size = 0;
        char *buffer = nullptr;

        buffer_size += sizeof(int);
        for (const auto &p : this->cluster)
        {
            buffer_size += sizeof(int) * sizeof(int);
            buffer_size += p.second.size() * sizeof(int);
        }

        buffer = new char[buffer_size];

        // pack the map into the buffer
        int position = 0;
        int num_keys = cluster.size();
        MPI_Pack(&num_keys, 1, MPI_INT, buffer, buffer_size, &position, comm);
        for (const auto &p : cluster)
        {
            int key = p.first;
            MPI_Pack(&key, 1, MPI_INT, buffer, buffer_size, &position, comm);
            int value_size = p.second.size();
            MPI_Pack(&value_size, 1, MPI_INT, buffer, buffer_size, &position, comm);
            MPI_Pack(p.second.data(), value_size, MPI_INT, buffer, buffer_size, &position, comm);
        }

        // broadcast the size of the buffer to all processes
        MPI_Bcast(&buffer_size, 1, MPI_INT, 0, comm);

        // allocate the buffer on the non-root processes
        if (rank != 0)
        {
            buffer = new char[buffer_size];
        }

        // broadcast the buffer to all processes
        MPI_Bcast(buffer, buffer_size, MPI_PACKED, 0, comm);

        // unpack the buffer into the map on the non-root processes
        if (rank != 0)
        {
            int position = 0;
            int num_keys;
            MPI_Unpack(buffer, buffer_size, &position, &num_keys, 1, MPI_INT, comm);
            for (int i = 0; i < num_keys; i++)
            {
                int key;
                MPI_Unpack(buffer, buffer_size, &position, &key, 1, MPI_INT, comm);
                int value_size;
                MPI_Unpack(buffer, buffer_size, &position, &value_size, 1, MPI_INT, comm);
                int *value = new int[value_size];
                MPI_Unpack(buffer, buffer_size, &position, value, value_size, MPI_INT, comm);
                cluster[key] = std::vector<int>(value, value + value_size);
                delete[] value;
            }
        }

        // print the map on each process
        for (const auto &p : cluster)
        {
            std::cout << "Process " << rank << ": "
                      << p.first << " -> ";
            for (int x : p.second)
            {
                std::cout << x << " ";
            }
            std::cout << std::endl;
        }
    }
}

void Dataset::initAssignCluster(int *center, int *dataPoints, std::map<int, std::vector<int>> cluster)
{

    for (int j = 0; j < this->numberOfClusters; j++)
    {
        for (int i = 0; i < 25; i++)
        {
            this->cluster[center[j]].push_back(dataPoints[i]);
        }
    }
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