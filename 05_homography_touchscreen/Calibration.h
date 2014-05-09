////////////////////////////////////////////////////////////////////////////////
//
// Performs calibration and provides all necessary matrices.
//
// Author: Patrick Lühne (2012)
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __CALIBRATION_H
#define __CALIBRATION_H

#include <opencv2/core/core.hpp>

#include <vector>
#include <string>

////////////////////////////////////////////////////////////////////////////////
//
// Calibration
//
////////////////////////////////////////////////////////////////////////////////

class Calibration
{
	public:
		Calibration();

		void restart();
		bool hasTerminated() const;

		void loop(const cv::Mat &rgbImage, const cv::Mat &depthImage);

		void handleMouseClick(int x, int y, int flags);

		const cv::Mat &physicalToProjector() const;
		const cv::Mat &projectorToPhysical() const;

		const cv::Mat &physicalToCamera() const;
		const cv::Mat &cameraToPhysical() const;

	protected:
		void calibrate(const cv::Mat &rgbImage);
		void calibrateProjector();
		void calibrateCamera(const cv::Mat &rgbImage);

		void computeHomography();

		bool m_hasTerminated;

		cv::Mat m_calibrationImage;

		std::vector<std::string> m_circleNames;

		bool m_isProjectorCalibrated;
		int m_numberOfProjectorCoordinates;

		// The 4 points for calibrating the projector
		std::vector<cv::Point2f> m_projectorCoordinates;

		bool m_isCameraCalibrated;
		int m_numberOfCameraCoordinates;

		// The 4 points for calibrating the camera
		std::vector<cv::Point2f> m_cameraCoordinates;

		// Matrices to convert between physical and projector space
		cv::Mat m_physicalToProjector;
		cv::Mat m_projectorToPhysical;

		// Matrices to convert between physical and camera space
		cv::Mat m_physicalToCamera;
		cv::Mat m_cameraToPhysical;
};

#endif
