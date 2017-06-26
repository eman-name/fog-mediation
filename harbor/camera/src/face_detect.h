#pragma once

#include "common.h"

#include <opencv2/core.hpp>

bool init_face_detect();

cv::Point2f get_face_marker(size_t index);

std::vector<cv::Rect> detect_faces(const cv::Mat& img);
std::vector<cv::Point> get_face_markers(const cv::Mat& img, const cv::Rect& bb);
void draw_face_markers(cv::Mat& image, const cv::Rect& bb, const std::vector<cv::Point>& markers);
