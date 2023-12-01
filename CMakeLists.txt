# Define the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Define the project name and the language
project(MyProject CXX)

# Define the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define the executable name
set(EXE main)

# Define the source files
file(GLOB SRC src/*.cpp)

# Define the library path
set(LIB lib/)

# Define the include path
include_directories(${LIB})

# Add the executable
add_executable(${EXE} main.cpp ${SRC})
