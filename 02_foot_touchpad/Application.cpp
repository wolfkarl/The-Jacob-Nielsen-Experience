////////////////////////////////////////////////////////////////////////////////
//
// Basic class organizing the application
//
// Authors: Stephan Richter (2011) and Patrick Lühne (2012)
//
////////////////////////////////////////////////////////////////////////////////

#include "Application.h"

#include "DepthCamera.h"
#include "DepthCameraException.h"
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
//
// Application
//
////////////////////////////////////////////////////////////////////////////////

void Application::processFrame()
{
	///////////////////////////////////////////////////////////////////////////
	//
	// To do:
	//
	// This method will be called every frame of the camera. Insert code here in
	// order to fulfill the assignment. These images will help you doing so:
	//
	// * m_rgbImage: The image of the Kinect's RGB camera
	// * m_depthImage: The image of the Kinects's depth sensor
	// * m_outputImage: The image in which you can draw the touch circles.
	//
	///////////////////////////////////////////////////////////////////////////

	// Sample code brightening up the depth image to make it visible
	m_depthImage *= 32;

	// save depthimage to temporary buffer and convert it to 8bit so 
	// openCV doesn't crash. Shoutout to Team EpicHigh5
	m_depthImage.copyTo(m_working);
	m_working.convertTo(m_working, CV_8UC1, 0.01);
	
	/* with normal tripod setup:

		# floor: 250
		# shoe touching floor: 230
		# everything above that: 220+

	*/

	int thresh_upper = 225;
	int thresh_lower = 240;


	cv::threshold(m_working, m_working, thresh_lower, 0, 4); // set the floor and everything farther away than the floor to black 
	cv::threshold(m_working, m_working, thresh_upper, 0, 3); // also, set everything to black that's to far away from the floor

	// now all thats left is feet touching the floor

	// first we have to declare an array of arrays to store our contours

	std::vector<std::vector<cv::Point>> contours; 
	
	// then we look for contours
	
	cv::findContours(m_working, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	// now we try to find the biggest contour...

	double maxContourSize = 0;
	double currContourSize;
	int maxContourIndex;
	int i;


	// ... of course only if we have found any

	if(contours.size() > 0)
	{
		for (i = 0; i < contours.size(); i++) {

			currContourSize = cv::contourArea(contours[i]);

			if (currContourSize > maxContourSize) {

				maxContourSize = currContourSize;
				maxContourIndex = i;

			}

		}
	}

	std::vector<cv::Point> maxContour = contours[maxContourIndex]; 
	std::cout << maxContour.size() << "\n";


	// small contours are probably just noise. only proceed if the contour is large
	if (maxContour.size() > 100)
	{
		// fitting ellipses

		cv::RotatedRect foot = cv::fitEllipse(maxContour);
		circle(m_working, foot.center, 20, cv::Scalar(100,150,200,0), 2);
		

	}


	m_working.copyTo(m_outputImage);

}

////////////////////////////////////////////////////////////////////////////////

Application::Application()
{
	m_isFinished = false;

	try
	{
		m_depthCamera = new DepthCamera;
	}
	catch (DepthCameraException)
	{
		m_isFinished = true;
		return;
	}

    // open windows
	cv::namedWindow("output", 1);
	cv::namedWindow("depth", 1);
	cv::namedWindow("raw", 1);

    // create work buffer
	m_rgbImage = cv::Mat(480, 640, CV_8UC3);
	m_depthImage = cv::Mat(480, 640, CV_16UC1),
	m_outputImage = cv::Mat(480, 640, CV_8UC1);
}

////////////////////////////////////////////////////////////////////////////////

Application::~Application()
{
	if (m_depthCamera)
		delete m_depthCamera;
}

////////////////////////////////////////////////////////////////////////////////

void Application::loop()
{
	// Check for key input
	int key = cv::waitKey(20);

	switch (key)
	{
		case 's':
			makeScreenshots();
			break;

		case 'c':
			clearOutputImage();
			break;

		case 'q':
			m_isFinished = true;
	}

	// Grab new images from the Kinect's cameras
	m_depthCamera->frameFromCamera(m_rgbImage, m_depthImage, CV_16UC1);

	// Process the current frame
	processFrame();

	// Display the images
	cv::imshow("raw", m_rgbImage);
	cv::imshow("depth", m_depthImage);
	cv::imshow("output", m_outputImage);
}

////////////////////////////////////////////////////////////////////////////////

void Application::makeScreenshots()
{
	cv::imwrite("raw.png", m_rgbImage);
	cv::imwrite("depth.png", m_depthImage);
	cv::imwrite("output.png", m_outputImage);
}

////////////////////////////////////////////////////////////////////////////////

void Application::clearOutputImage()
{
	cv::rectangle(m_outputImage, cv::Point(0, 0), cv::Point(640, 480),
				  cv::Scalar::all(0), CV_FILLED);
}

////////////////////////////////////////////////////////////////////////////////

bool Application::isFinished()
{
	return m_isFinished;
}
