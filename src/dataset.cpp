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

    // int *nodeArray = new int[4];
    std::vector<int> nodeArray;
    nodeArray.resize(4);

    std::vector<int> sendBuffer;
    if (rank == 0)
    {

        sendBuffer.resize(100);

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

    MPI_Bcast(nodeArray.data(), 4, MPI_INT, root, comm);

    // receive buffer
    std::vector<int> recvBuffer;
    recvBuffer.resize(25);

    MPI_Scatter(sendBuffer.data(), this->numberOfPoints / number_of_processors, MPI_INT, recvBuffer.data(), this->numberOfPoints / number_of_processors, MPI_INT, root, comm);

    std::map<int, std::vector<int>> assignCluster = this->initAssignCluster(nodeArray[rank], recvBuffer, cluster);

    std::vector<int> gatherData;
    gatherData.resize(100);

    std::map<int, std::vector<int>> gatherCluster;

    const int size = 4;

    // use a vector to store the buffer
    std::vector<char> buffer;

    // pack the map into the buffer
    int position = 0;
    int num_keys = assignCluster.size();
    buffer.resize(sizeof(int) + (num_keys) * (sizeof(int) * 2));

    MPI_Pack(&num_keys, 1, MPI_INT, buffer.data(), buffer.size(), &position, comm);
    for (const auto &p : assignCluster)
    {
        int key = p.first;
        if (p.second.size() > 0)
        {
            MPI_Pack(&key, 1, MPI_INT, buffer.data(), buffer.size(), &position, comm);
            int value_size = p.second.size();
            MPI_Pack(&value_size, 1, MPI_INT, buffer.data(), buffer.size(), &position, comm);
            buffer.resize(buffer.size() + value_size * sizeof(int));
            MPI_Pack(p.second.data(), value_size, MPI_INT, buffer.data(), buffer.size(), &position, comm);
        }
    }

    // gather the size of the buffer from all processes
    std::vector<int> gather_buffer_size(size);

    int buffer_size = buffer.size();

    MPI_Gather(&buffer_size, 1, MPI_INT, gather_buffer_size.data(), 1, MPI_INT, 0, comm);

    // use a smart pointer to manage the memory for the gather buffer on the root process
    std::vector<int> gather_buffer;
    int total_buffer_size = 0;

    if (rank == 0)
    {
        for (int i = 0; i < size; i++)
        {
            total_buffer_size += gather_buffer_size[i];
        }
        gather_buffer.resize(total_buffer_size);
    }

    // gather the buffer from all processes
    std::vector<int> displs(size);
    displs[0] = 0;
    for (int i = 1; i < size; i++)
    {
        displs[i] = displs[i - 1] + gather_buffer_size[i - 1];
    }

    MPI_Gatherv(buffer.data(), buffer.size(), MPI_PACKED, gather_buffer.data(), gather_buffer_size.data(), displs.data(), MPI_PACKED, 0, comm);

    if (rank == 0)
    {
        int position1 = 0;
        for (int i = 0; i < size; i++)
        {
            int num_keys1;
            MPI_Unpack(gather_buffer.data(), gather_buffer_size[i], &position1, &num_keys1, 1, MPI_INT, comm);

            for (int j = 0; j < num_keys1 - 1; j++)
            {
                int key;
                MPI_Unpack(gather_buffer.data(), gather_buffer_size[i], &position1, &key, 1, MPI_INT, comm);
                int value_size;
                MPI_Unpack(gather_buffer.data(), gather_buffer_size[i], &position1, &value_size, 1, MPI_INT, comm);
                std::vector<int> value(value_size);

                MPI_Unpack(gather_buffer.data(), gather_buffer_size[i], &position1, value.data(), value_size, MPI_INT, comm);

                gatherCluster[key] = value;
            }
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

    // TODO: put the data points in the proper cluster untile there is no changes happen in out clusters
    // TODO: calculate the mean
}

std::map<int, std::vector<int>> Dataset::initAssignCluster(int center, std::vector<int> dataPoints, std::map<int, std::vector<int>> cluster)
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