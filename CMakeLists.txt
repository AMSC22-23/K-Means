cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

# set C++ version requirements
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set variable for project name
set(PROJECT_NAME
    kmeans
)

# set libraries names (no libraries without templates)
set(LIBRARIES
)

# set project name and version
project(
  ${PROJECT_NAME}
  VERSION 1.0
  LANGUAGES CXX)

# set include directory
include_directories(/lib)

set(SOURCES 
    ./main.cpp

)

# create executable
add_executable(${PROJECT_NAME}
    ${SOURCES} 
)

find_package(MPI REQUIRED)
# link libraries to project
if(MPI_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC  ${LIBRARIES} MPI::MPI_CXX)
else()
    target_link_libraries(${PROJECT_NAME} ${LIBRARIES})
endif()

# Set the output directory for the executable file
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})