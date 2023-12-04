# Set the minimum version of CMake required to build the project
cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(main VERSION 1.0)

# Find the MPI package
find_package(MPI REQUIRED)

# Add the executable target
add_executable(main main.cpp)

# Add the src and lib directories to the include path
target_include_directories(main PRIVATE src lib)

# Link the MPI libraries to the executable
target_link_libraries(main MPI::MPI_CXX)

# Set the C++ standard to 11
set_target_properties(main PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Set the output directory for the executable file
set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})