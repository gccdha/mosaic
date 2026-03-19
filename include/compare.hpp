#pragma once

#include <opencv2/opencv.hpp>

int brightness_diff(cv::Scalar source, cv::Scalar target);
int cost(cv::Mat, cv::Mat, char method = ' ');
int diff(cv::Scalar, cv::Scalar, char method = ' ');

