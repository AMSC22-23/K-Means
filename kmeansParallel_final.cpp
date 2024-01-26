#include <iostream>
#include <mpi.h>
#include <stddef.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>

struct Points
{
    double x,y,mindistance;
    int p_cluster;

    Points ():x(0.0),y(0.0),p_cluster(-1),mindistance(__DBL_MAX__){
    }
    Points (double x, double y) : x(x), y(y),p_cluster(-1),mindistance(__DBL_MAX__){

    }

    double calculate_distance (double x, double y)
    {
        double distance = (x - this->x) * (x - this->x) + (y - this->y) * (y - this->y);
        return distance;
    }

};

//reading from file
std::vector<Points> readmyfile()
{
    std::vector<Points> listofpoints;
    std::string line;
    std::ifstream file ("my_in_file.csv");
    while (getline(file,line))
    {
        double x,y;
        std::string numberasstring;
        std::stringstream streamof(line);
        getline (streamof,numberasstring,',');
        x = stof(numberasstring);
        getline (streamof,numberasstring,'\n');
        y = stof(numberasstring);
        listofpoints.emplace_back(x,y);
    }
    return listofpoints;
}

void kmeansclustring (std::vector<Points>* input_vector,int k)
{
    int mpi_rank,mpi_size;
    std::vector<Points> result_vec (input_vector->size()); //output vector for gatherv
    MPI_Comm mpi_comm = MPI_COMM_WORLD;
    MPI_Comm_rank(mpi_comm,&mpi_rank);
    MPI_Comm_size(mpi_comm,&mpi_size);

    const int nitems = 4;
    int blocklengths [4] = {1,1,1,1};
    MPI_Datatype types [4] = {MPI_DOUBLE,MPI_DOUBLE,MPI_DOUBLE,MPI_INT};
    MPI_Datatype mpi_points_type;
    MPI_Aint offsets[4];

    offsets[0] = offsetof(Points, x);
    offsets[1] = offsetof(Points, y);
    offsets[2] = offsetof(Points, mindistance);
    offsets[3] = offsetof(Points, p_cluster);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_points_type);
    MPI_Type_commit(&mpi_points_type);

    std::vector<Points> centerpoints;
    int centervectorsize;
    if (mpi_rank == 0)
    {
        srand(time(0));
        int n = input_vector->size();
        for (int i =0;i<k;++i)
        {
            centerpoints.emplace_back((*input_vector)[rand()%n]);
        }
        centervectorsize = centerpoints.size();
    }
    //resizing the vector for other nodes.
    MPI_Bcast(&centervectorsize, 1, MPI_INT, 0 , mpi_comm);
    if (mpi_rank != 0)
    {
        centerpoints.resize (centervectorsize);
    }
    //now every node has the vector that contains the centerpoints.
    MPI_Bcast(centerpoints.data(),centervectorsize, mpi_points_type, 0, mpi_comm);
    
    std::vector<int> n_points_per_node(mpi_size);
    std::vector<int> displacement (mpi_size,0);
    if (mpi_rank == 0)
    {
        int chunk = input_vector->size()/mpi_size;
        int rest = input_vector->size()%mpi_size;
        for (int i = 0; i<mpi_size;++i)
        {
            n_points_per_node[i] = i < rest ? chunk+1 : chunk;
            if (i>0)
                displacement[i] = displacement[i-1] + n_points_per_node[i-1];
        }
    }
    //sending chunks of data to each node.
    int local_size;
    MPI_Scatter (n_points_per_node.data(), 1, MPI_INT, &local_size, 1, MPI_INT, 0, mpi_comm);
    std::vector<Points> local_chunk_of_data(local_size);
    MPI_Scatterv (input_vector->data(),n_points_per_node.data(),displacement.data(),
    mpi_points_type, local_chunk_of_data.data(),local_size,mpi_points_type,0,mpi_comm);

    //number of iterations goes here
    for (int iter=0;iter<1000;++iter)
    {
        //iterate and calculate distance to each center for every point;
        for (std::vector<Points>::iterator cen = centerpoints.begin(); cen<centerpoints.end(); ++cen)
        {
            int cid = cen - centerpoints.begin();
            for (std::vector<Points>::iterator points = local_chunk_of_data.begin(); points<local_chunk_of_data.end(); ++points)
            {
                double distance = points->calculate_distance (centerpoints[cid].x, centerpoints[cid].y);
                if (distance < points->mindistance)
                {
                    points->mindistance = distance;
                    points->p_cluster = cid;
                }
            }
        }
        std::vector<int> clustermembercounter;
        std::vector <double> sumofxvalues;
        std::vector <double> sumofyvalues;
        for (int i = 0 ; i<k;++i)
        {
            clustermembercounter.emplace_back(0);
            sumofxvalues.emplace_back(0.0);
            sumofyvalues.emplace_back(0.0);
        }

        //now we calculate cluster averages and numbers.
        for (std::vector<Points>::iterator points = local_chunk_of_data.begin(); points < local_chunk_of_data.end(); ++points)
        {
            clustermembercounter[points->p_cluster] ++;
            sumofxvalues[points->p_cluster] += points->x;
            sumofyvalues[points->p_cluster] += points->y;

            points->mindistance = __DBL_MAX__;
        }
        std::vector <int> total_number_of_members_in_cluster(k);
        std::vector <double> total_x_of_cluster(k) ;
        std::vector <double> total_y_of_cluster(k) ;
        for (int i=0;i<k;i++)
        {
            MPI_Allreduce(&clustermembercounter[i], &total_number_of_members_in_cluster[i], 1,MPI_INT, MPI_SUM,mpi_comm );
            MPI_Allreduce(&sumofxvalues[i], &total_x_of_cluster[i], 1,MPI_DOUBLE, MPI_SUM,mpi_comm );
            MPI_Allreduce(&sumofyvalues[i], &total_y_of_cluster[i], 1,MPI_DOUBLE, MPI_SUM,mpi_comm );
        }

        //calculate new centers
        for (std::vector<Points>::iterator centers = centerpoints.begin(); centers<centerpoints.end(); ++centers)
        {
            int cid = centers - centerpoints.begin();
            centerpoints[cid].x = total_x_of_cluster[cid] / total_number_of_members_in_cluster[cid];
            centerpoints[cid].y = total_y_of_cluster[cid] / total_number_of_members_in_cluster[cid];
        }
    }

    MPI_Gatherv (local_chunk_of_data.data(), local_size, mpi_points_type, result_vec.data(),n_points_per_node.data(),
    displacement.data(), mpi_points_type, 0, mpi_comm);

    if (mpi_rank==0)
    {
        //now we write to file
        std::ofstream outputfile;
        outputfile.open ("outputfile.csv");
        outputfile <<"x,y,c"<<std::endl;
        for (std::vector<Points>::iterator iv = result_vec.begin(); iv <result_vec.end() ; ++iv)
        {
            outputfile << iv->x <<","<< iv->y<< ","<< iv->p_cluster << std::endl;
        }
        outputfile.close();
    }
   
    //to avoid memory leaks.
    MPI_Type_free(&mpi_points_type);
}

int main()
{
    MPI_Init(nullptr,nullptr);
        
    std::vector<Points> points = readmyfile();
    //number of clusters goes here
    kmeansclustring(&points,5);

    MPI_Finalize();
}