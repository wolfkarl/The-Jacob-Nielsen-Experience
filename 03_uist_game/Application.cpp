////////////////////////////////////////////////////////////////////////////////
//
// Basic class organizing the application
//
// Authors: Patrick Lühne (2012)
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

////////////////////////////////////////////////////////////////////////////////
//
// Application
//
////////////////////////////////////////////////////////////////////////////////

void Application::processFrame()
{
	////////////////////////////////////////////////////////////////////////////
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
	////////////////////////////////////////////////////////////////////////////

	// Sample code brightening up the depth image to make it visible
	m_depthImage *= 32;
}

////////////////////////////////////////////////////////////////////////////////

void Application::handleSkeletonTracked(XnUInt16 userID)
{
	////////////////////////////////////////////////////////////////////////////
	//
	// To do:
	//
	// Handle skeleton tracking here:
	//
	// * userID: The ID of the user being tracked.
	//
	////////////////////////////////////////////////////////////////////////////
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

    // open windows
	cv::namedWindow("depth", CV_WINDOW_AUTOSIZE);
	cv::namedWindow("raw", CV_WINDOW_AUTOSIZE);
	cv::namedWindow("UIST 2001 game", CV_WINDOW_AUTOSIZE);

    // create work buffer
	m_rgbImage = cv::Mat(480, 640, CV_8UC3);
	m_depthImage = cv::Mat(480, 640, CV_16UC1);
	m_renderImage = cv::Mat(480, 480, CV_8UC3);
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
}

////////////////////////////////////////////////////////////////////////////////

void Application::loop()
{
	try
	{
		if (DepthCamera::hasInstance())
		{
			// Grab new images from the Kinect's cameras
			DepthCamera::instance()->frameFromCamera(m_rgbImage, m_depthImage, CV_16UC1);

			// Process the current frame
			processFrame();
		}
	}
	catch (DepthCameraException)
	{

	}

	if (m_gameClient->game())
		m_gameClient->game()->render(m_renderImage);

	// Display the images
	cv::imshow("raw", m_rgbImage);
	cv::imshow("depth", m_depthImage);
	cv::imshow("UIST 2001 game", m_renderImage);

	// Check for key input
	int key = cv::waitKey(1);

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

		case ' ':
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
