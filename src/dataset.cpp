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
    // Define the root process
    int root = 0;
    int *nodeArray = NULL;
    nodeArray = new int[100];

    int *sendBuffer;

    // send operation
    if (rank == 0)
    {
        sendBuffer = new int[100];

        for (int i = 0; i < this->numberOfPoints; i++)
        {
            sendBuffer[i] = this->pointList[i];
        }

        // brodcast the number of clusters to all the nodes
        //        MPI_Bcast(&this->numberOfClusters, 1, MPI_INT, root, comm);
        // brodcast the number of data points to all the nodes
        //        MPI_Bcast(&this->pointList, this->pointList.size(), MPI_INT, root, comm);

        // randomly select the center points for each cluster
        for (int i = 0; i < this->numberOfClusters; i++)
        {
            int random = std::rand() % 4;
            nodeArray[i] = this->pointList[random];
        }
    }
    // recv operation
    else
    {
        // receive number of broadcasted clusters
        // MPI_Bcast(&this->numberOfClusters, 1, MPI_INT, root, comm);

        // receive number of broadcasted data points
        //        MPI_Bcast(&this->numberOfPoints, 1, MPI_INT, root, comm);
    }

    int *recvBuffer = new int[25];

    MPI_Scatter((void *)sendBuffer, 25, MPI_INT, (void *)recvBuffer, 25, MPI_INT, root, comm);

    for (int i = 0; i < 25; i++)
    {
        printf("The index %d number is %d", i, recvBuffer[i]);
        printf("\n");
    }
}
