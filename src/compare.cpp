#include "compare.hpp"
#include <cstdlib>
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

int brightness_diff(cv::Scalar source, cv::Scalar target){
  return std::abs((source[0]+source[1]+source[2]) - (target[0]+target[1]+target[2]));
}

std::vector<int> generate_matrix(std::vector<cv::Mat>& sources, std::vector<cv::Mat>& targets, bool exclude_identical){
  const int rows = targets.size();
  const int cols = sources.size();
  const int tile_height = targets[0].rows;
  const int tile_width = targets[0].cols;

 //WARN: using INT_MAX isn't perfect here. It could still be chosen assuming the cost gets large enough, but is generally fine (guranteed to work I think for images with < ~2.8 million total pixels) 
  std::vector<int> output(rows*cols, INT_MAX);
  // #pragma omp parallel for collapse(2)
  for(int i = 0; i < rows; i++){
    for(int j = 0; j < cols; j++){
      if(!(j%rows == i && exclude_identical))
        output[i*cols+j] = cost(sources[j], targets[i], tile_height, tile_width);
    }
  }
  return output;
}

//Calculate difference score between two squares using average or the summed pixelwise difference
int cost(cv::Mat source, cv::Mat target, const int tile_height, const int tile_width, char method){
  switch(method){
    case 'a':{ // average pixel value
      cv::Scalar s_mean = cv::mean(source);
      cv::Scalar t_mean = cv::mean(target);
      return diff(s_mean, t_mean);
      break;}
    case 'e': // edge mask (canny) TODO:
      return -1;
      break;
    default:{  // summed pixelwise difference
      int out = 0;
      for(int i=0; i < tile_height; i++){
        for(int j=0; j < tile_width; j++){
          out+=diff(source.at<cv::Vec3b>(i,j),target.at<cv::Vec3b>(i,j));
        }
      }
      return out;
      break;}
  }
  return -100;
}

//calculate the difference between two pixels using brightness or manhatten distance
int diff(cv::Scalar source, cv::Scalar target, char method){
  switch(method){
    case 'b': //brightness
      return std::abs((source[0]+source[1]+source[2])-(target[0]+target[1]+target[2]));
      break;
    default: //manhatten distance
      return std::abs(source[0]-target[0])+abs(source[1]-target[1])+abs(source[2]-target[2]);
      break;
  }
}

