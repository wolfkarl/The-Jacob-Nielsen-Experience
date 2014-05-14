////////////////////////////////////////////////////////////////////////////////
//
// Basic class organizing the application
//
// Author: Patrick Lühne (2012)
//
////////////////////////////////////////////////////////////////////////////////

#include "Application.h"

#include <boost/thread.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

#include "game/GameClient.h"
#include "game/GameServer.h"
#include "game/Game.h"
#include "game/GameUnit.h"

#include "DepthCamera.h"
#include "DepthCameraException.h"
#include "Calibration.h"
#include "OpenCVUtils.h"

////////////////////////////////////////////////////////////////////////////////
//
// Application
//
////////////////////////////////////////////////////////////////////////////////

void Application::warpImage()
{
	////////////////////////////////////////////////////////////////////////////
	//
	// To do (assignment #5):
	//
	// In this method, you have to warp the image in order to project it on the
	// floor so that it appears undistorted.
	//
	// * m_renderImage: The image you have to distort in a way that it appears
	//                  undistorted on the floor
	// * m_calibration: The calibration class giving you access to the matrices
	//                  you have computed.
	//
	////////////////////////////////////////////////////////////////////////////

	// save renderimage to temporary mat
	cv::Mat tmp;
	m_renderImage.copyTo(tmp);

	// to the matmobile, let's go!
	cv::warpPerspective(tmp, m_renderImage, m_calibration->physicalToProjector(), m_renderImage.size());
}

////////////////////////////////////////////////////////////////////////////////

void Application::processFrame()
{
	////////////////////////////////////////////////////////////////////////////
	//
	// To do (assignment #2):
	//
	// This method will be called every frame of the camera. Insert code here in
	// order to recognize touch. These images will help you doing so:
	//
	// * m_rgbImage: The image of the Kinect's RGB camera
	// * m_depthImage: The image of the Kinects's depth sensor
	// * m_gameImage: The undistorted image that the game renders
	// * m_renderImage: The final image distorted in a way that it appears
	//                  undistorted on the floor.
	//
	////////////////////////////////////////////////////////////////////////////
	
	// Sample code brightening up the depth image to make it visible
	m_depthImage *= 32;
	bool new_input = false;

	// save depthimage to temporary buffer and convert it to 8bit so 
	// openCV doesn't crash. Shoutout to Team EpicHigh5
	m_depthImage.copyTo(m_working);
	//cv::perspectiveTransform(m_depthImage, m_working, m_calibration->cameraToPhysical());
	m_working.convertTo(m_working, CV_8UC1, 0.006, 0); // very important magic number
	

	if (!initialized)
	{
		std::cout << "Initialize!\n\n";
		m_working.copyTo(m_base);
		initialized = 1;
	}

	// generic shit: remove floor from image
	cv::absdiff(m_base, m_working, m_working);

	// lighten shit up
	m_working *= 2;

	int thresh_upper = 50;
	int thresh_lower = 10;
	cv::threshold(m_working, m_working, thresh_upper, 0, 4);
	cv::threshold(m_working, m_working, thresh_lower, 0, 3);

	// now all thats left is feet touching the floor

	// first we have to declare an array of arrays to store our contours

	std::vector<std::vector<cv::Point>> contours; 

	// then we look for contours
	cv::findContours(m_working, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	// now we try to find the biggest contour...

	double maxContourSize = 0;
	double currContourSize;
	int maxContourIndex= -1;


	// ... of course only if we have found any

	if(contours.size() > 0)
	{
			int i;
		for (i = 0; i < contours.size(); i++) {

			currContourSize = cv::contourArea(contours[i]);

			if (currContourSize > maxContourSize) {

				maxContourSize = currContourSize;
				maxContourIndex = i;

			}

		}
	}

	// have we found a suitable contour?
	if (maxContourIndex >= 0)
	{
		std::vector<cv::Point> maxContour = contours[maxContourIndex]; 

		//std::cout << maxContour.size() << "\n";


		// small contours are probably just noise. only proceed if the contour is large
		if (maxContour.size() > 100)
		{
			// fitting ellipses


			cv::Point input_proj;
			cv::RotatedRect foot = cv::fitEllipse(maxContour);
			//std::cout << foot.center.x << " " << foot.center.y << "--> ";
			input = m_calibration->cameraToPhysical() * foot.center;
			input_proj = m_calibration->physicalToProjector() *  m_calibration->cameraToPhysical() * foot.center;
			//circle(m_renderImage, foot.center, 10, cv::Scalar(100,150,200,0), 2);
			//std::cout << foot.center.x << " " << foot.center.y << "\n";
			circle(m_renderImage, input, 10, cv::Scalar(200,100,200,0), 2);
			new_input=1;
			std::cout << input.x << "|" << input.y << " --- ";
		}
	} 

	////////////////////////////////////////////////////////////////////////////
	//
	// To do (assignment #5 and #6):
	//
	// After having recognized touch points, select the according game units and
	// control them.
	//
	////////////////////////////////////////////////////////////////////////////
	


	// loop of destiny (one loop to rule them all)

	int i = 0;
	int m = 5; // no. of units. being generic is so fun!
	int tolerance = 40;

	for (i = 0; i<m; i++)
	{
		GameUnitPtr u = m_gameClient->game()->unitByIndex(i);

		if(new_input)
		{
			// was this unit just selected or deselected?
			int distance = cv::norm(u->position() - input);
			//std::cout << distance << " ";
			std::cout << u->position().x << "|" << u->position().y << " ";
			if (distance < tolerance)
			{
				// toggle highlightedness of unit
				std::cout << "You tapped Unit no. " << (i+1) << "(distance: " << distance << ")\n";
				//u->setHighlighted(!u->isHighlighted());
				m_gameClient->game()->highlightUnit(i, !u->isHighlighted());
			}
		}

		// everybody selected, please come to the last known foot position
		// or you won't get any pudding
		if(u->isHighlighted() && u->isLiving())
		{
			//float angle = std::atan(float(u->position().y - input.y) / (input.x - u->position().x ));
		float angle;

		double wurst = u->position().y - input.y;
		double conchita = input.x - u->position().x;
		if(conchita != 0 && wurst != 0){ angle = atan(wurst/conchita);
		if (conchita < 0) { angle += M_PI; 	} else if (wurst < 0) { angle += 2*M_PI;}
		} else if (wurst == 0){ if(conchita > 0){angle = M_PI;} else { 	angle = 0.0;}
		} else { if(wurst > 0){	angle = 3 * M_PI / 2;} else {angle = M_PI / 2;}
}
			m_gameClient->game()->moveUnit(i, angle, 1.0f);
		}



	}
	std::cout << "\n";
}

////////////////////////////////////////////////////////////////////////////////

void Application::handleSkeletonTracked(XnUInt16 userID)
{
	////////////////////////////////////////////////////////////////////////////
	//
	// To do (assignments #3 and #4):
	//
	// Handle skeleton tracking here:
	//
	// * userID: The ID of the user being tracked.
	//
	////////////////////////////////////////////////////////////////////////////

	xn::SkeletonCapability skeletonCapability
		= DepthCamera::instance()->userGenerator().GetSkeletonCap();

	// Access joint positions like this:
	// skeletonCapability->GetSkeletonJointPosition(...);
}

////////////////////////////////////////////////////////////////////////////////

Application::Application(int argc, char *argv[])
{
	m_isFinished = false;

	m_gameServer = new GameServer;
	m_gameClient = new GameClient;

	m_gameServer->run();
	m_gameServer->loadGame(1);

	boost::this_thread::sleep(boost::posix_time::milliseconds(100));

	m_gameClient->run();

	if (argc > 1)
		m_gameClient->connectToServer(argv[1]);
	else
	{
		m_gameClient->connectToServer("127.0.0.1");

		std::cout << "[Info] Connected to localhost (to connect to another "
			<< "server, pass a server address as the first argument)."
			<< std::endl;
	}

	try
	{
		DepthCamera::instance()->onSkeletonTracked.connect(
			boost::bind(&Application::handleSkeletonTracked, this, _1));
	}
	catch (DepthCameraException)
	{

	}

	// Start the calibration
	m_calibration = new Calibration;

    // Create necessary images
	m_rgbImage = cv::Mat(480, 640, CV_8UC3);
	m_depthImage = cv::Mat(480, 640, CV_16UC1);
	m_gameImage = cv::Mat(480, 480, CV_8UC3);
	m_renderImage = cv::Mat(600, 800, CV_8UC3);
	m_working = cv::Mat(480, 640, CV_8UC1);

	// initialize initialize
	initialized = false;
}

////////////////////////////////////////////////////////////////////////////////

Application::~Application()
{
	m_gameClient->stop();
	m_gameServer->stop();

	DepthCamera::deleteInstance();

	if (m_gameClient)
		delete m_gameClient;

	if (m_gameServer)
		delete m_gameServer;

	if (m_calibration)
		delete m_calibration;
}

////////////////////////////////////////////////////////////////////////////////

void Application::loop()
{
	try
	{
		// Grab new images from the Kinect's cameras
		if (DepthCamera::hasInstance())
			DepthCamera::instance()->frameFromCamera(m_rgbImage, m_depthImage, CV_16UC1);
	}
	catch (DepthCameraException)
	{

	}

	cv::flip(m_rgbImage, m_rgbImage, 1);
	cv::flip(m_depthImage, m_depthImage, 1);

	int key;

	// If projector and camera aren't calibrated, do this and nothing else
	if (!m_calibration->hasTerminated())
	{
		m_calibration->loop(m_rgbImage, m_depthImage);

		key = cv::waitKey(1);

		if (key == 'q')
			m_isFinished = true;

		return;
	}

	if (m_gameClient->game())
		m_gameClient->game()->render(m_gameImage);

	// Copy the game image into the finally rendered image
	m_renderImage = cv::Mat::zeros(600, 800, CV_8UC3);
	cv::Mat renderRegionOfInterest = m_renderImage(cv::Rect(0, 0, 480, 480));
	m_gameImage.copyTo(renderRegionOfInterest);

	// Undistort the image
	warpImage();

	// Process the current frame
	processFrame();

	// Display the image
	cv::imshow("UIST 2001 game", m_renderImage);

	// Check for key input
	key = cv::waitKey(1);

	switch (key)
	{
		case 'p':
			makeScreenshots();
			break;

		case 'q':
			m_isFinished = true;
			break;

		case 'w':
			// Testing: Move the first unit north
			if (m_gameClient->game())
				m_gameClient->game()->moveUnit(0, M_PI / 2, 1.0f);
			break;

		case 'a':
			// Testing: Move the first unit west
			if (m_gameClient->game())
				m_gameClient->game()->moveUnit(0, M_PI, 1.0f);
			break;

		case 's':
			// Testing: Move the first unit south
			if (m_gameClient->game())
				m_gameClient->game()->moveUnit(0, 3 * M_PI / 2, 1.0f);
			break;

		case 'd':
			// Testing: Move the first unit east
			if (m_gameClient->game())
				m_gameClient->game()->moveUnit(0, 0.0f, 1.0f);
			break;

		case 'e':
			// Testing: Stop the first unit
			if (m_gameClient->game())
				m_gameClient->game()->moveUnit(0, 0.0f, 0.0f);
			break;

		case 'h':
			// Testing: Highlight the first unit
			if (m_gameClient->game())
				m_gameClient->game()->highlightUnit(0, true);
			break;

		case 'u':
			// Testing: Unhighlight the first unit
			if (m_gameClient->game())
				m_gameClient->game()->highlightUnit(0, false);
			break;

		case ' ':
			// Start the game
			m_gameServer->startGame();
			break;

		case 'c':
			// Recalibrate projector and camera
			m_calibration->restart();
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////

void Application::makeScreenshots()
{
	cv::imwrite("raw.png", m_rgbImage);
	cv::imwrite("depth.png", m_depthImage);
	cv::imwrite("game.png", m_renderImage);
}

////////////////////////////////////////////////////////////////////////////////

bool Application::isFinished()
{
	return m_isFinished;
}
