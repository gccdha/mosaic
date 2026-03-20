//This file contains functions related to reading, creating, and writing images

#include "images.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/base.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <string>
#include <tuple>


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
    case 0:
      break;
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
//can choose to include or not include rotations and reflections using symetries
//bits in order are 0cw, 90cw, 180cw, 270cw, R+0cw, R+90cw, R+180cw, R+270cw
//the first AREA elements in squares will be the normal images. this will be followed by 90, 180, 270
// and reflected 0, 90, 180, and 270 rotated squares.
void disect(cv::Mat img, std::vector<cv::Mat>& tiles, bool symetries[8], const int height, const int width, const int tile_height, const int tile_width){
  assert((tile_height == tile_width) && "Tiles must be square"); //TODO: Implement non-square tiles
  const int area = height*width;
  std::vector<cv::Mat> raw_tiles(area, cv::Mat(cv::Size(tile_width,tile_height),CV_8UC3,cv::Scalar(0,0,0)));

  //first, loop over all the tiles
  for(int i=0; i < height; i++){
    int yindex = i*tile_height; //y pixel index
    for(int j = 0; j < width; j++){
      int xindex = j*tile_width; //x pixel index
      for(int k=0; k < tile_height; k++){
        for(int l=0; l < tile_width; l++){
          raw_tiles[i*width+j].at<cv::Vec3b>(k,l)=img.at<cv::Vec3b>(yindex+k, xindex+l); 
          //squares[i][j].at<cv::Vec3b>(k,l)=img.at<cv::Vec3b>(i, j);
        }
      }
    }
  }

  //add reflections and rotations
  for(int i = 0; i<8;i++){
    if(symetries[i]){
      bool ref = i>=4;
      int rot = i%4;
      for(int j=0; j<area; j++){
        tiles.push_back(transform(raw_tiles[j],ref,rot)); 
      }
    }
  }

  return;
}


cv::Mat stitch(std::vector<cv::Mat>& tiles, const int width, const int height, int border_size){
  const int tile_height = tiles[0].rows + 2*border_size;
  const int tile_width = tiles[0].cols + 2*border_size;
  const int num_tiles = width*height;

  //create blank canvas
  cv::Mat canvas(height * tile_height, width*tile_width, tiles[0].type());

  for(int i = 0; i < num_tiles; i++){
    int row = i / width;
    int col = i % width;
    
    //create mask over the correct region
    cv::Rect region(col*tile_width,  row*tile_height, tile_width, tile_height);

    //TODO: add border option
    //
    
    cv::Mat img;
    tiles[i].copyTo(img);

    if(border_size) cv::copyMakeBorder(tiles[i], img,
                                  border_size, border_size, border_size, border_size,
                                  cv::BORDER_CONSTANT, cv::Scalar(255,0,0));
    
    //copy to region
    img.copyTo(canvas(region));
  }

  return canvas;
}

std::vector<cv::Mat> select_images(std::vector<int>& matching, std::vector<cv::Mat>& sources, const int jobs){
  std::vector<cv::Mat> selected(jobs);

  for(int i = 0; i < matching.size(); i++){
    if(matching[i] >= 0) selected[matching[i]] = sources[i];
  }

  return selected;
}


cv::Mat visualize_matching(std::vector<int>& matching, const int height, const int width, const int tile_height, const int tile_width){
  //text parameters
  int font = cv::FONT_HERSHEY_SIMPLEX;
  double scale = 1.0;
  int thickness = 2;
  int baseline = 0;
  std::vector<cv::Mat> sources;

  for(int i = 0; i < height * width; i++){
    //create black tile and add text
    cv::Mat tile = cv::Mat(tile_height, tile_width, CV_8UC3, cv::Scalar(0,0,0));
    
    if(matching[i] >= 0){
      std::stringstream ss;
      ss << i;
      std::string num = ss.str();
      //get font size and location (top left corner)
      cv::Size text_size = cv::getTextSize(num, font, scale,thickness, &baseline);
      cv::Point text_org((tile_width - text_size.width)/2, (tile_height + text_size.height)/2);
      cv::putText(tile, num, text_org, font, scale, cv::Scalar(255,255,255), thickness);
    }

    sources.push_back(tile);
  }
  std::vector<cv::Mat> tiles = select_images(matching, sources, height*width);
  return stitch(tiles, width, height, 1);
}




cv::Rect calculate_crop(const int cols, const int rows, const int width, const int height, CropMethod method){
  unsigned int x,y;
  switch(method.vertical){
    case START:
      y = 0;
    case MIDDLE:
      y = (rows - height)/2;
    case END:
      y = rows - height;
  }
  switch(method.horizontal){
  case START:
      x = 0;
  case MIDDLE:
      x = (cols - width)/2;
  case END:
      x = cols - width;
  }

  return cv::Rect(x,y,width,height);
}

//crop both images to the min of their heights and width, and then crop to be a multiple of the tile size
//crop methods can be chosen for both images but both use fully centered cropping by default
//TODO: add option to not crop to each others size (source must have more tiles than target)
void crop_to_match(cv::Mat& source, cv::Mat& target, const int tile_width, const int tile_height, CropMethod source_method, CropMethod target_method){
  //Crop each image so that they are exactly the same size
  const int min_height = std::min(source.rows, target.rows);
  const int min_width = std::min(source.cols, target.cols);
  cv::Rect source_crop = calculate_crop(source.cols, source.rows, min_width, min_height, source_method);
  cv::Rect target_crop = calculate_crop(target.cols, target.rows, min_width, min_height, target_method);
  source = source(source_crop);
  target = target(target_crop);

  //Crop each image to the closest tile
  const int x_extra = source.cols % tile_width;
  const int y_extra = source.rows % tile_height;
  cv::Rect source_align = calculate_crop(source.cols, source.rows, source.cols - x_extra, source.rows - y_extra, source_method);
  cv::Rect target_align = calculate_crop(target.cols, target.rows, target.cols - x_extra, target.rows - y_extra, target_method);

  source = source(source_align);
  target = target(target_align);
}

