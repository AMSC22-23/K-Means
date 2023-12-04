# K-Means

This is a C++ project that implements the K-means algorithm using MPI.

### Requirements
- CMake 3.10 or higher
- MPI library and compiler
- C++ compiler that supports C++11 standard

## Build
To build the project, follow these steps:

- Create an empty directory for the build files, such as build/
- Navigate to the build directory and run cmake <path> where <path> is the directory where the CMakeLists.txt file is located
- Run cmake --build . to build the executable
- Alternatively, you can use the generated build files directly with the corresponding tool, such as make
## Run
To run the project, follow these steps:

- Navigate to the build directory and run mpirun -np <k> ./main where <k> is the number of clusters and processes
- The program will output the execution time and the final centroids with their counts

```
mkdir build
cd build
cmake ..
make
cd ..
mpirun -np 2 ./main
```
