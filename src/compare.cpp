#include "compare.hpp"
#include <cstdlib>
#include <opencv2/opencv.hpp>

int brightness_diff(cv::Scalar source, cv::Scalar target){
  return std::abs((source[0]+source[1]+source[2]) - (target[0]+target[1]+target[2]));
}

//Calculate difference score between two squares using average or the summed pixelwise difference
int cost(cv::Mat source, cv::Mat target, char method, const int SQSIZE){
  switch(method){
    case 'a':{ // average pixel value
      cv::Scalar s_mean = cv::mean(source);
      cv::Scalar t_mean = cv::mean(target);
      return diff(s_mean, t_mean);
      break;}
    case 'e': // edge mask (canny) TODO
      return -1;
      break;
    default:{  // summed pixelwise difference
      int out = 0;
      for(int i=SQSIZE-1; i>=0; i--){
        for(int j=SQSIZE-1; j>=0;j--){
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

