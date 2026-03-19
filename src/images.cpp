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

//Reflect and/or rotate an image:
//if reflect == true, reflect over the x axis
//rotate rotates 90, 180 or 270 degrees for 1, 2, and 3 respectively
//reflection is applied before rotation
cv::Mat transform(cv::Mat img, bool reflect, int rotate){
  if(reflect){
    // reflect over x axis
    cv::flip(img, img, 0);
  }
  switch(rotate){
    case 1:
      cv::rotate(img, img, cv::ROTATE_90_CLOCKWISE);
      break;
    case 2:
      cv::rotate(img, img, cv::ROTATE_180);
      break;
    case 3:
      cv::rotate(img, img, cv::ROTATE_90_COUNTERCLOCKWISE);
      break;
  }
  return img.clone();
}

//disect the input image into a number of squares based on the global parameters
//can choose to include or not include rotations and reflections
//the first AREA elements in squares will be the normal images. this will be followed by 90, 180, 270
// and reflected 0, 90, 180, and 270 rotated squares.
void disect(cv::Mat img, cv::Mat squares[], bool symetries, const int SQHEIGHT, const int SQWIDTH, const int SQSIZE, const int AREA){
  //first, loop over all the macro-squares
  for(int i=SQHEIGHT-1; i>=0; i--){
    int yindex = i*SQSIZE;
    for(int j=SQWIDTH-1; j>=0; j--){
      int xindex = j*SQSIZE;
      cv::Mat tmp(cv::Size(SQSIZE,SQSIZE),CV_8UC3,cv::Scalar(0,0,0));
      int square = i*SQWIDTH+j;
      squares[square]=tmp;
      //then, loop over all pixels in the macro-square
      for(int k=SQSIZE-1; k>=0; k--){
        for(int l=SQSIZE-1;l>=0; l--){
          squares[square].at<cv::Vec3b>(k,l)=img.at<cv::Vec3b>(yindex+k, xindex+l); // TODO: Is in neccesary to copy each pixel likes this?
          //squares[i][j].at<cv::Vec3b>(k,l)=img.at<cv::Vec3b>(i, j);
        }
      }
    }
  }
  if(!symetries) return;

  //add reflections and rotations
  for(int i = 1; i<8;i++){
    int k=i*AREA;
    int ref = (i)/4;
    int rot = (i)%4;
    for(int j=0; j<AREA; j++){
      squares[j+k]=transform(squares[j],ref,rot); 
    }
  }
  return;
}
