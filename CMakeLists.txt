cmake_minimum_required(VERSION 3.10)
project(MyOpenCVProject)

# Find OpenMP package
find_package(OpenMP REQUIRED)

# Set compiler flags for OpenMP
if(OpenMP_CXX_FOUND)
    target_compile_options(${PROJECT_NAME} PRIVATE ${OpenMP_CXX_FLAGS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenMP_CXX_LIBRARIES})
endif()


# Find OpenCV
find_package(OpenCV REQUIRED)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})

# Add your executable
add_executable(my_project main.cpp)

# Link OpenCV libraries
target_link_libraries(my_project ${OpenCV_LIBS})

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g")

