#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <ctime>

struct Point
{
    double x, y;
};

class Cluster
{
public:
    std::vector<Point> points;
    Point centroid;

    double distance(Point a, Point b)
    {
        return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
    }

    void updateCentroid()
    {
        double sumX = 0, sumY = 0;
        for (Point p : points)
        {
            sumX += p.x;
            sumY += p.y;
        }
        centroid.x = sumX / points.size();
        centroid.y = sumY / points.size();
    }
};

void generateCSV(std::string filename, int numPoints)
{
    std::srand(std::time(0));
    std::ofstream file(filename);
    for (int i = 0; i < numPoints; ++i)
    {
        double x = (double)std::rand() / RAND_MAX;
        double y = (double)std::rand() / RAND_MAX;
        file << x << "," << y << "\n";
    }
}

std::vector<Point> readCSV(std::string filename)
{
    std::vector<Point> data;
    std::ifstream file(filename);
    std::string line;
    while (getline(file, line))
    {
        std::stringstream ss(line);
        std::string cell;
        Point p;
        getline(ss, cell, ',');
        p.x = std::stod(cell);
        getline(ss, cell, ',');
        p.y = std::stod(cell);
        data.push_back(p);
    }
    return data;
}

void kmeans(std::vector<Point> &data, int k)
{
    std::vector<Cluster> clusters(k);
    for (int i = 0; i < k; ++i)
    {
        clusters[i].centroid = data[i];
    }
    // sdflsdf;lssdf;lmsdfs;lmdfsd;sdfs;l,sds;lmvsd;lmds;lmsd
    // sdfmsdflklkmxccvxcvlkmdssdvxclkmvsclmxcvl lkmsdvxclkmsl

    for (int i = 0; i < 1000; i++)
    {

        // finding closest cluster for each point
        for (Point &p : data)
        {
            int closest = 0;
            // first calculated distance from the center
            double closestDist = clusters[0].distance(p, clusters[0].centroid);
            for (int i = 1; i < k; ++i)
            {
                double dist = clusters[i].distance(p, clusters[i].centroid);
                if (dist < closestDist)
                {
                    closest = i;
                    closestDist = dist;
                }
            }

            // check if current point is already in the proper cluster

            if (clusters[closest].points.empty() || p.x != clusters[closest].points.back().x || p.y != clusters[closest].points.back().y)
            {
                clusters[closest].points.push_back(p);
            }
        }
        for (Cluster &c : clusters)
        {
            c.updateCentroid();
        }
    }
    // Write the results to a CSV file
    std::ofstream file("results.csv");
    for (int i = 0; i < k; ++i)
    {
        for (Point p : clusters[i].points)
        {
            file << p.x << "," << p.y << "," << i << "\n";
        }
    }
}

void plotWithGnuplot()
{
    std::string gnuplotCommand = "gnuplot -p -e \"";
    gnuplotCommand += "set datafile separator ',';";
    gnuplotCommand += "plot 'results.csv' using 1:2:3 with points palette notitle";
    gnuplotCommand += "\"";

    system(gnuplotCommand.c_str());
}

int main()
{
    generateCSV("data.csv", 1000);
    std::vector<Point> data = readCSV("data.csv");
    kmeans(data, 5);

    plotWithGnuplot();
}