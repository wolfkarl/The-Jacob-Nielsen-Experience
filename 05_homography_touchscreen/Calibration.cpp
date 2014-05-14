////////////////////////////////////////////////////////////////////////////////
//
// Performs calibration and provides all necessary matrices.
//
// Author: Patrick Lühne (2012)
//
////////////////////////////////////////////////////////////////////////////////

#include "Calibration.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

#include <limits>

#include "OpenCVUtils.h"

////////////////////////////////////////////////////////////////////////////////
//
// Calibration
//
////////////////////////////////////////////////////////////////////////////////

void Calibration::computeHomography()
{
	////////////////////////////////////////////////////////////////////////////
	//
	// To do (assignment #5):
	//
	// Compute the homography matrices here in order to transform points between
	// the different spaces. These variables will help you doing so:
	//
	// * m_projectorCoordinates: Vector of 4 points you have clicked to
	//                           calibrate the projector
	// * m_cameraCoordinates: Vector of 4 points you have clicked to calibrate
	//                        the camera.
	//
	// At the end, these matrices should to be set:
	//
	// * m_physicalToProjector
	// * m_projectorToPhysical
	// * m_physicalToCamera
	// * m_cameraToPhysical
	//
	////////////////////////////////////////////////////////////////////////////

	// jakob nielsen up in dis bitch!

	// invent physical coordinates and save them to list
	std::vector<cv::Point2f> physicalCoordinates;
	physicalCoordinates.push_back(cv::Point2f(0,480)); // bottom left
	physicalCoordinates.push_back(cv::Point2f(480,480)); // bottom right
	physicalCoordinates.push_back(cv::Point2f(480,0)); // top right
	physicalCoordinates.push_back(cv::Point2f(0,0)); // top left


	// californibration
	
	// phys <--> proj
	m_physicalToProjector = cv::getPerspectiveTransform(physicalCoordinates, m_projectorCoordinates);
	m_projectorToPhysical = cv::getPerspectiveTransform(m_projectorCoordinates, physicalCoordinates);

	// phys <--> cam
	m_physicalToCamera = cv::getPerspectiveTransform(physicalCoordinates, m_cameraCoordinates);
	m_cameraToPhysical = cv::getPerspectiveTransform(m_cameraCoordinates, physicalCoordinates);
	
	


}

////////////////////////////////////////////////////////////////////////////////

void mouseCallback(int event, int x, int y, int flags, void *pointer);

////////////////////////////////////////////////////////////////////////////////

Calibration::Calibration()
{
	restart();

	m_circleNames.push_back("bottom-left (red)");
	m_circleNames.push_back("bottom-right (green)");
	m_circleNames.push_back("top-right (blue)");
	m_circleNames.push_back("top-left (black)");

	m_calibrationImage = cv::Mat(600, 800, CV_8UC3);
}

////////////////////////////////////////////////////////////////////////////////

void Calibration::restart()
{
	m_hasTerminated = false;
	m_isProjectorCalibrated = false;
	m_isCameraCalibrated = false;

	m_numberOfProjectorCoordinates = 0;
	m_numberOfCameraCoordinates = 0;

	m_projectorCoordinates.clear();
	m_cameraCoordinates.clear();

	cv::destroyWindow("UIST 2001 game");

	cv::namedWindow("calibration", CV_WINDOW_NORMAL);

	cv::setMouseCallback("calibration", mouseCallback, this);
}

////////////////////////////////////////////////////////////////////////////////

bool Calibration::hasTerminated() const
{
	return m_hasTerminated;
}

////////////////////////////////////////////////////////////////////////////////

void Calibration::loop(const cv::Mat &rgbImage, const cv::Mat &depthImage)
{
	// Reset the calibration wizard image
	m_calibrationImage = cv::Mat::zeros(600, 800, CV_8UC3);

	// Run the calibration wizard
	calibrate(rgbImage);

	// Show the calibration wizard
	if (!m_hasTerminated)
		cv::imshow("calibration", m_calibrationImage);
}

////////////////////////////////////////////////////////////////////////////////

void Calibration::calibrate(const cv::Mat &rgbImage)
{
	// First, calibrate the projector
	if (!m_isProjectorCalibrated)
	{
		calibrateProjector();
		return;
	}

	// Then, calibrate the camera
	if (!m_isCameraCalibrated)
	{
		calibrateCamera(rgbImage);
		return;
	}

	// If both are calibrated, compute the homographies
	computeHomography();

	// Finally, hide the calibration wizard and show the UIST game instead
	cv::destroyWindow("calibration");

	cv::namedWindow("UIST 2001 game", CV_WINDOW_NORMAL);

	m_hasTerminated = true;
}

////////////////////////////////////////////////////////////////////////////////

void Calibration::calibrateProjector()
{
	std::stringstream info;
	info << "Click on the " << m_circleNames[m_numberOfProjectorCoordinates]
		<< " circle.";

	cv::putText(m_calibrationImage, info.str(), cv::Point2i(16, 32),
				cv::FONT_HERSHEY_SIMPLEX, 1.0f, cv::Scalar(192, 192, 192), 1.0f,
				CV_AA);
}

////////////////////////////////////////////////////////////////////////////////

void Calibration::calibrateCamera(const cv::Mat &rgbImage)
{
	cv::Mat destinationRegionOfInterest
		= m_calibrationImage(cv::Rect(0, 0, rgbImage.cols, rgbImage.rows));

	// Show the RGB image in order to click on it
	rgbImage.copyTo(destinationRegionOfInterest);

	std::stringstream info;
	info << "Click on the " << m_circleNames[m_numberOfCameraCoordinates]
		<< " circle.";

	cv::putText(m_calibrationImage, info.str(), cv::Point2i(16, 32),
				cv::FONT_HERSHEY_SIMPLEX, 1.0f, cv::Scalar(192, 192, 192), 1.0f,
				CV_AA);
}

////////////////////////////////////////////////////////////////////////////////

void Calibration::handleMouseClick(int x, int y, int flags)
{
	// If the projector is not calibrated, save the clicked point as a projector
	// calibration point
	if (m_numberOfProjectorCoordinates < 4)
	{
		m_projectorCoordinates.push_back(cv::Point2f(x, y));
		m_numberOfProjectorCoordinates++;

		if (m_numberOfProjectorCoordinates >= 4)
		{
			m_isProjectorCalibrated = true;

			std::cout << "[Info] Finished calibrating the projector."
				<< std::endl;
		}

		return;
	}

	// If the projector is calibrated, but not the camera, save the clicked
	// point as a camera calibration point
	if (m_numberOfCameraCoordinates < 4)
	{
		m_cameraCoordinates.push_back(cv::Point2f(x, y));
		m_numberOfCameraCoordinates++;

		if (m_numberOfCameraCoordinates >= 4)
		{
			m_isCameraCalibrated = true;

			std::cout << "[Info] Finished calibrating the camera." << std::endl;
		}

		return;
	}
}

////////////////////////////////////////////////////////////////////////////////

const cv::Mat &Calibration::physicalToProjector() const
{
	return m_physicalToProjector;
}

////////////////////////////////////////////////////////////////////////////////

const cv::Mat &Calibration::projectorToPhysical() const
{
	return m_projectorToPhysical;
}

////////////////////////////////////////////////////////////////////////////////

const cv::Mat &Calibration::physicalToCamera() const
{
	return m_physicalToCamera;
}

////////////////////////////////////////////////////////////////////////////////

const cv::Mat &Calibration::cameraToPhysical() const
{
	return m_cameraToPhysical;
}

////////////////////////////////////////////////////////////////////////////////

void mouseCallback(int event, int x, int y, int flags, void *pointer)
{
	if (event != CV_EVENT_LBUTTONDOWN)
		return;

	Calibration* calibration = static_cast<Calibration*>(pointer);

	if (calibration)
		calibration->handleMouseClick(x, y, flags);
}
