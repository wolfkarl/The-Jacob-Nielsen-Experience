////////////////////////////////////////////////////////////////////////////////
//
// Basic class organizing the application
//
// Authors: Stephan Richter (2011) and Patrick Lühne (2012)
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __APPLICATION_H
#define __APPLICATION_H

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// Forward declarations
class DepthCamera;

////////////////////////////////////////////////////////////////////////////////
//
// Application
//
////////////////////////////////////////////////////////////////////////////////

class Application
{
	public:
		Application();
		~Application();

		void loop();

		////////////////////////////////////////////////////////////////////////
		//
		// To do: Insert your code here
		//
		////////////////////////////////////////////////////////////////////////
		void processFrame();

		void makeScreenshots();
		void clearOutputImage();

		bool isFinished();
		bool initialized;
		

	protected:
		DepthCamera *m_depthCamera;

		cv::Mat m_rgbImage;
		cv::Mat m_depthImage;
		cv::Mat m_outputImage;
		cv::Mat m_base;

		cv::Mat m_working;
		cv::Mat m_empty;

		bool m_isFinished;
};

#endif // __APPLICATION_H
