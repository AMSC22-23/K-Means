#include "../lib/dataset.h"
#include "../lib/json.hpp"
using json = nlohmann::json;

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

// Define a struct to represent a person
struct Person
{
    std::string name;
    int age;
};

// Define a function to serialize a person to a JSON object
json serialize(const Person &person)
{
    // Create a JSON object
    json j;
    // Add the name and age of the person to the JSON object
    j["name"] = person.name;
    j["age"] = person.age;
    // Return the JSON object
    return j;
}

// Define a function to deserialize a JSON object to a person
Person deserialize(const json &j)
{
    // Create a person to store the result
    Person person;
    // Get the name and age of the person from the JSON object
    person.name = j["name"].get<std::string>();
    person.age = j["age"].get<int>();
    // Return the person

    std::cout << person.age << " " << person.name << std::endl;

    return person;
}

void Dataset::createClusters(int rank, MPI_Comm comm)
{
    const int number_of_processors = 4;
    const int number_of_iterations = 20;
    // Define the root process
    int ROOT = 0;

    int ierr;
    // Serialize and prepare data for scatter on the root process

    // Serialize and prepare data for scatter on the root process
    json j;

    // Create a vector of person objects

    std::vector<Person> persons = {{"Alice", 30}, {"Bob", 25}, {"Charlie", 28}, {"David", 32}};
    // Serialize each person object to a JSON object
    for (const auto &person : persons)
    {
        j.push_back(serialize(person));
    }

    // Broadcast the length of the JSON array to all processes
    int json_len = j.dump().length();
    ierr = MPI_Bcast(&json_len, 1, MPI_INT, ROOT, comm);
    if (ierr != MPI_SUCCESS)
    {
        std::cerr << "MPI_Bcast failed\n";
        MPI_Finalize();
    }

    // Calculate the size of each chunk based on the length of the JSON array and the number of processes
    int chunk_size = json_len / 4;
    // Allocate a buffer to store the JSON array on each process
    char *json_buf = new char[chunk_size + 1];

    // Scatter the JSON array to all processes
    ierr = MPI_Scatter(j.dump().c_str(), chunk_size + 1, MPI_CHAR, json_buf, chunk_size + 1, MPI_CHAR, ROOT, comm);
    if (ierr != MPI_SUCCESS)
    {
        std::cerr << "MPI_Scatter failed\n";
        delete[] json_buf;
        MPI_Finalize();
    }

    // Deserialize the JSON array to a vector of person objects on each process
    std::vector<Person> deserialized_persons;
    // Calculate the start and stop indices of the for loop based on the rank and the chunk size
    int start = rank * chunk_size;
    int stop = start + chunk_size;
    for (int i = start; i < stop - 1; i++)
    {
        std::stringstream ss;
        ss << json_buf[i];
        deserialized_persons.push_back(deserialize(json::parse(ss)));
    }

    // Delete the buffer
    delete[] json_buf;

    // Print the deserialized vector of person objects on each process

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