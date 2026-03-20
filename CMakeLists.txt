cmake_minimum_required(VERSION 3.10)  # Set minimum CMake version

project(mosaic)


# clangd LSP support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# allow c++20 features
set(CMAKE_CXX_STANDARD 20)

# Find OpenCV package (assuming OpenCV is installed)
find_package(OpenCV REQUIRED)

# Find OpenMP package
find_package(OpenMP REQUIRED)

# Add source files
set(SOURCES
  src/main.cpp 
  src/images.cpp 
  src/assignment.cpp 
  src/compare.cpp
)
add_compile_options(-Wall -Wextra -Wpedantic -g)
add_executable(mosaic ${SOURCES})  # Define the target executable

# add includes
target_include_directories(mosaic
  PRIVATE ${PROJECT_SOURCE_DIR}/include 
)

# Apply OpenMP flags (only if OpenMP is found)
if(OpenMP_CXX_FOUND)
  target_compile_options(mosaic PRIVATE ${OpenMP_CXX_FLAGS})  # Add OpenMP compile flags
  target_link_libraries(mosaic PRIVATE ${OpenMP_CXX_LIBRARIES})  # Link OpenMP libraries
  message(STATUS "OpenMP found")
else()
  message(WARNING "OpenMP not found")
endif()

# Link OpenCV libraries to the project
target_link_libraries(mosaic PRIVATE ${OpenCV_LIBS})

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
  set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()
