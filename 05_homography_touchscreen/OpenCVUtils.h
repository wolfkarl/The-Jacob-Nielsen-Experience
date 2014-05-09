#ifndef __OPENCV_UTILS_H
#define __OPENCV_UTILS_H

#include <opencv2/core/core.hpp>

cv::Point2f operator*(cv::Mat M, const cv::Point2f &p);

#endif
