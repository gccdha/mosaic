//This file contains functions related to reading, creating, and writing images

#include "images.hpp"
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <string>


bool read_image(std::string file, cv::Mat& output){
  std::cout << "reading " << file << '\n';
  output = cv::imread(file);
  if(output.empty()){
    std::cerr << "Error reading file " << file << '\n';
    return false;
  } else return true;
}
