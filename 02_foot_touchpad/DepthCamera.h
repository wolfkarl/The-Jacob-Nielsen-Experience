////////////////////////////////////////////////////////////////////////////////
//
// Class grabbing frames from the depth camera
//
// Authors: Stephan Richter (2011) and Patrick Lühne (2012)
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __DEPTH_CAMERA_H
#define __DEPTH_CAMERA_H

// OpenNI
#include <XnCppWrapper.h>
#include <XnOpenNI.h>

// OpenCV
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

////////////////////////////////////////////////////////////////////////////////
//
// DepthCamera
//
////////////////////////////////////////////////////////////////////////////////

class DepthCamera
{
	public:
		DepthCamera();
		~DepthCamera();

		static const std::string s_sampleXMLPath;

		// Reads an RGB and depth frame from the camera. The format of the depth
		// frame can be either CV_16UC1 or CV_8UC3. If CV_8UC3, the depth is
		// encoded with the high byte in the green channel and the low byte in
		// the blue channel.
		int frameFromCamera(cv::Mat &rgbImage, cv::Mat &depthImage,
							int depthType = CV_16UC1);

		// Opens RGB and depth video files.
		bool loadVideo(const std::string &rgbFile,
					   const std::string &depthFile);

		// Reads a frame from the video file opened with loadVideo. The format
		// of the images depends on the file format. Returns true if reading
		// succeeds.
		bool frameFromVideo(cv::Mat &rgbFile, cv::Mat &depthFile);

		// Reads RGB and depth images from files. The format of the images
		// depends on the file format.
		static bool frameFromFile(std::string rgbFile, cv::Mat &rgbImage,
								 std::string depthFile, cv::Mat &depthImage);

		// Converts a depth image from CV_8UC3 to CV_16UC1. It is assumed that
		// the depth is encoded with the high byte in the green channel and the
		// low byte in the blue channel.
		static void convertDepth_8UC3_to_16UC1(const cv::Mat &depth8,
											   cv::Mat &depth16);

		// Getters & setters
		xn::DepthGenerator &depthGenerator() { return m_depthGenerator; }
		xn::ImageGenerator &imageGenerator() { return m_imageGenerator; }

	protected:
		xn::Context m_context;
		xn::DepthGenerator m_depthGenerator;
		xn::ImageGenerator m_imageGenerator;
		xn::DepthMetaData m_depthMetaData;
		xn::ImageMetaData m_imageMetaData;

		cv::VideoCapture m_rgbReader;
		cv::VideoCapture m_depthReader;

		static void throwException(std::string description, XnStatus status);

		// Converts the depth data from an OpenNI depth generator to an OpenCV
		// Mat. Returns a cv::Mat(480, 640, CV_16UC1) in depthImage.
		static void convertDepthToMat_16UC1(
			const xn::DepthMetaData &depthMetaData, cv::Mat &depthImage);

		// Converts the depth data from an OpenNI depth generator to an OpenCV
		// Mat. Returns a cv::Mat(480, 640, CV_8UC3) in depthImage.
		static void convertDepthToMat_8UC3(
			const xn::DepthMetaData &depthMetaData, cv::Mat &depthImage);

		// Converts the image data from an OpenNI image generator to an OpenCV
		// Mat. Returns a cv::Mat(480, 640, CV_8UC3) in rgbImage.
		static void convertRGBToMat(
			const xn::ImageMetaData &imageMetaData, cv::Mat &rgbImage);
};

#endif // DEPTH_CAMERA_H
