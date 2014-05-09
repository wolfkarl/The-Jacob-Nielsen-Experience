#include "OpenCVUtils.h"

////////////////////////////////////////////////////////////////////////////////

cv::Point2f operator*(cv::Mat M, const cv::Point2f &p)
{
	cv::Mat src(3, 1, CV_64F);

	src.at<double>(0, 0) = p.x;
	src.at<double>(1, 0) = p.y;
	src.at<double>(2, 0) = 1.0;

	cv::Mat dst = M * src;

	return cv::Point2f(dst.at<double>(0, 0), dst.at<double>(1, 0));
}
