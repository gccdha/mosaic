#pragma once

#include <opencv2/opencv.hpp>

int brightness_diff(cv::Scalar source, cv::Scalar target);
std::vector<int> generate_matrix(std::vector<cv::Mat>&, std::vector<cv::Mat>&, bool);
int cost(cv::Mat, cv::Mat, const int, const int, char method = ' ');
int diff(cv::Scalar, cv::Scalar, char method = ' ');

