#pragma once

#include "common.h"

#include <opencv2/core.hpp>

bool init_face_features();
std::vector<float> get_face_features(const cv::Mat& image);
