#pragma once

#include <opencv2/core/mat.hpp>
#include <string>

bool read_image(std::string file, cv::Mat& output);
cv::Mat transform(const cv::Mat&, bool reflect = 0, int rotate = 0);
void disect(cv::Mat, std::vector<cv::Mat>&, bool[8], const int, const int, const int, const int);
std::vector<cv::Mat> select_images(std::vector<int>& matching, std::vector<cv::Mat>& sources, const int jobs);
cv::Mat visualize_matching(std::vector<int>& matching, const int height, const int width, const int tile_height, const int tile_width);
cv::Mat stitch(std::vector<cv::Mat>& tiles, const int width, const int height, int border_size = 0);

//start is top or left and end is bottom or right
enum CropMethod1D {START, MIDDLE, END};
struct CropMethod {
  CropMethod1D vertical = MIDDLE;
  CropMethod1D horizontal = MIDDLE;
};
void crop_to_match(cv::Mat& source, cv::Mat& target, const int tile_width, const int tile_height, CropMethod source_methd = {}, CropMethod target_method = {});



