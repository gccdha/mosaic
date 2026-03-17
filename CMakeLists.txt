cmake_minimum_required(VERSION 3.10)  # Set minimum CMake version

project(mosaic)  # Your project name


# clangd LSP support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# allow c++20 features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Find OpenCV package (assuming OpenCV is installed)
find_package(OpenCV REQUIRED)

# Find OpenMP package
find_package(OpenMP REQUIRED)

# Add source files
add_executable(mosaic main.cpp)  # Define the target executable

# Apply OpenMP flags (only if OpenMP is found)
if(OpenMP_CXX_FOUND)
    target_compile_options(mosaic PRIVATE ${OpenMP_CXX_FLAGS})  # Add OpenMP compile flags
    target_link_libraries(mosaic PRIVATE ${OpenMP_CXX_LIBRARIES})  # Link OpenMP libraries
endif()

# Link OpenCV libraries to the project
target_link_libraries(mosaic PRIVATE ${OpenCV_LIBS})

# Debug
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

