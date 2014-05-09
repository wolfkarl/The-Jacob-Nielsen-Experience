////////////////////////////////////////////////////////////////////////////////
//
// Basic class organizing the application
//
// Author: Patrick Lühne (2012)
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __APPLICATION_H
#define __APPLICATION_H

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// Forward declarations
class DepthCamera;
class GameClient;
class GameServer;
class Calibration;

// OpenNI
#include <XnCppWrapper.h>
#include <XnOpenNI.h>

////////////////////////////////////////////////////////////////////////////////
//
// Application
//
////////////////////////////////////////////////////////////////////////////////

class Application
{
	public:
		Application(int argc, char *argv[]);
		~Application();

		void loop();

		void warpImage();
		void processFrame();
		void handleSkeletonTracked(XnUInt16 userID);

		void makeScreenshots();
		void clearOutputImage();

		bool isFinished();

	protected:
		GameClient *m_gameClient;
		GameServer *m_gameServer;

		cv::Mat m_rgbImage;
		cv::Mat m_depthImage;
		cv::Mat m_gameImage;
		cv::Mat m_renderImage;

		Calibration *m_calibration;

		bool m_isFinished;
};

#endif
