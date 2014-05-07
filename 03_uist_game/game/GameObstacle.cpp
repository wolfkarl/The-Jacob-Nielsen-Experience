#include "GameObstacle.h"

#include <boost/bind.hpp>

#include <string>

#include <opencv2/imgproc/imgproc.hpp>

#include "GameNetworkInterface.h"
#include "Logging.h"

////////////////////////////////////////////////////////////////////////////////
//
// GameObstacle
//
////////////////////////////////////////////////////////////////////////////////

GameObstacle::GameObstacle(GameNetworkInterface *gameNetworkInterface)
	: Message(gameNetworkInterface)
{
	// Set up obstacle data for network transmission via messages
	registerMessageType(MESSAGE_GAME_OBSTACLE, &m_gameObstacleData,
		sizeof(GameObstacleData), UPDATE_FREQUENCY_ALWAYS);

	setRadius(64);
}

////////////////////////////////////////////////////////////////////////////////

void GameObstacle::render(cv::Mat &image)
{
	cv::Scalar color(128, 128, 128);

	cv::circle(image, cv::Point(x(), y()), radius(), color, CV_FILLED, CV_AA);
}

////////////////////////////////////////////////////////////////////////////////

void GameObstacle::setPosition(float x, float y)
{
	m_gameObstacleData.x = x;
	m_gameObstacleData.y = y;
}

////////////////////////////////////////////////////////////////////////////////

cv::Point GameObstacle::position() const
{
	return cv::Point(x(), y());
}

////////////////////////////////////////////////////////////////////////////////

float GameObstacle::x() const
{
	return m_gameObstacleData.x;
}

////////////////////////////////////////////////////////////////////////////////

float GameObstacle::y() const
{
	return m_gameObstacleData.y;
}

////////////////////////////////////////////////////////////////////////////////

void GameObstacle::setRadius(float radius)
{
	m_gameObstacleData.radius = radius;
}

////////////////////////////////////////////////////////////////////////////////

float GameObstacle::radius() const
{
	return m_gameObstacleData.radius;
}
