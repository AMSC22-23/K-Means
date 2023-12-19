//@note: pragma does not work like this
//       you either use `#pragma once`, where `once` is a keyword
//       or the standard header guard with the naming convention you prefer
#pragma DATASET_H

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <ctime>
#include <vector>
#include <mpi.h>

//@note: in structs everything is public, this often defeats the purpose of object oriented programming (OOP)
//       - Encapsulation is one of the fundamental principles of OOP. 
//         It involves bundling the data (attributes) and methods that operate on the data into a 
//         single unit (a class), and restricting access to the internal details of the class. 
//         Making everything public breaks encapsulation, allowing external code to directly 
//         manipulate the internal state of an object. This can lead to unintended side effects 
//         and makes it harder to reason about and maintain the code.
//       - By exposing all methods and attributes, you lose control over how the class is used. 
//         Users of the class might misuse or modify the internal state in ways that were not 
//         intended, leading to unexpected behavior.
//@note: it is very limiting that a point can have just one integer value, you should be able to 
//       handle a arbitrary number of dimensions
struct Point
{
    int id;
    int value;
    Point() : id(0), value(0){};
    Point(int pointId, int pointValue);
};

struct Cluster
{
    int id;
    int mean;
    std::vector<Point> data;

    Cluster(int id);
    //@note: you should take arguments by const reference instead of copy when they are large
    //       to avoid copying large objects (e.g. `data`)
    Cluster(int id, std::vector<Point> data);
    Cluster() : id(1), data(1, Point(0, 0)) {}
    // void calculateMean();
};

struct Dataset
{
    int numberOfPoints;
    int numberOfClusters;
    int maxIteration;
    std::vector<Point> pointList;
    std::vector<Cluster> clusterList;
    std::string filename;

    //@note: you should take arguments by const reference instead of copy when they are large
    //       to avoid copying large objects (e.g. `filename`)
    Dataset(int numberOfPoints, int numberOfClusters, int maxIteration, std::string filename);

    void generateData();
    void readData(std::string filename);
    //@note: data visualization is a different objective from the handling of the 
    //       data itself; it could be a good idea to decouple them
    void showDataPoints();
    void showTheClusterData();
    //@note: data clustering is a different objective from the handling of the 
    //       data itself; it could be a good idea to decouple them
    void createClusters(int rank, MPI_Comm comm);
};
