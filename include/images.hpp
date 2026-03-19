#pragma once

#include <opencv2/core/mat.hpp>
#include <string>

bool read_image(std::string file, cv::Mat& output);
cv::Mat transform(cv::Mat, bool reflect = 0, int rotate = 0);
void disect(cv::Mat, cv::Mat[], bool, const int, const int, const int, const int);


