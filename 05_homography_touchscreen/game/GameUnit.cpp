#include "GameUnit.h"

#include <boost/bind.hpp>

#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "GameNetworkInterface.h"
#include "GameObstacle.h"
#include "Logging.h"

////////////////////////////////////////////////////////////////////////////////
//
// GameUnit
//
////////////////////////////////////////////////////////////////////////////////

float GameUnit::s_maximalVelocity = 110.0f;
float GameUnit::s_maximalAcceleration = 1000.0f;
float GameUnit::s_brakeFactor = 10.0f;
float GameUnit::s_radius = 8.0f;

////////////////////////////////////////////////////////////////////////////////

GameUnit::GameUnit(GameNetworkInterface *gameNetworkInterface)
	: Message(gameNetworkInterface)
{
	// Set up unit data for network transmission via messages
	registerMessageType(MESSAGE_GAME_UNIT, &m_gameUnitData,
		sizeof(GameUnitData), UPDATE_FREQUENCY_ALWAYS);

	m_velocity = cv::Vec2f(0.0f, 0.0f);
	m_acceleration = cv::Vec2f(0.0f, 0.0f);

	setLiving(true);
	setArrived(false);
	setHighlighted(false);
}

////////////////////////////////////////////////////////////////////////////////

void GameUnit::render(cv::Mat &image)
{
	cv::Scalar color;

	if (hasArrived())
		color = cv::Scalar(255, 255, 255);
	else if (!isLiving())
		color = cv::Scalar(0, 0, 0);
	else if (owner() == ID_FIRST_CLIENT)
		color = cv::Scalar(192, 160, 0);
	else
		color = cv::Scalar(0, 64, 192);

	if (isHighlighted() && isLiving() && !hasArrived())
		cv::circle(image, cv::Point(x(), y()), s_radius + 4, cv::Scalar(224, 224, 224), CV_FILLED, CV_AA);

	cv::circle(image, cv::Point(x(), y()), s_radius, color, CV_FILLED, CV_AA);

	std::stringstream numberText;
	numberText << (int)number();

	cv::putText(image, numberText.str(),
				cv::Point(x() - s_radius / 2, y() + s_radius / 2),
				cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 255, 255), 1.25,
				CV_AA);
}

////////////////////////////////////////////////////////////////////////////////

void GameUnit::move(float timeDifference)
{
	if (!isLiving() || hasArrived())
	{
		m_velocity = cv::Vec2f(0.0f, 0.0f);
		m_acceleration = cv::Vec2f(0.0f, 0.0f);

		return;
	}

	// Brake the object slightly
	cv::Vec2f acceleration = m_acceleration - m_velocity * s_brakeFactor;

	m_velocity += acceleration * timeDifference;

	// If the speed exceeds the maximal speed, reset the speed to the maximum
	float velocityNorm = cv::norm(m_velocity);

	if (velocityNorm > s_maximalVelocity)
		m_velocity *= s_maximalVelocity / velocityNorm;

	m_gameUnitData.x += m_velocity[0] * timeDifference;
	m_gameUnitData.y += m_velocity[1] * timeDifference;
}

////////////////////////////////////////////////////////////////////////////////

void GameUnit::setPosition(float x, float y)
{
	m_gameUnitData.x = x;
	m_gameUnitData.y = y;
}

////////////////////////////////////////////////////////////////////////////////

cv::Point GameUnit::position() const
{
	return cv::Point(x(), y());
}

////////////////////////////////////////////////////////////////////////////////

float GameUnit::x() const
{
	return m_gameUnitData.x;
}

////////////////////////////////////////////////////////////////////////////////

float &GameUnit::x()
{
	return m_gameUnitData.x;
}

////////////////////////////////////////////////////////////////////////////////

float GameUnit::y() const
{
	return m_gameUnitData.y;
}

////////////////////////////////////////////////////////////////////////////////

float &GameUnit::y()
{
	return m_gameUnitData.y;
}

////////////////////////////////////////////////////////////////////////////////

void GameUnit::setNumber(uint8_t number)
{
	m_gameUnitData.number = number;
}

////////////////////////////////////////////////////////////////////////////////

uint8_t GameUnit::number()
{
	return m_gameUnitData.number;
}

////////////////////////////////////////////////////////////////////////////////

void GameUnit::setOwner(PlayerID owner)
{
	m_gameUnitData.owner = owner;
}

////////////////////////////////////////////////////////////////////////////////

PlayerID GameUnit::owner() const
{
	return m_gameUnitData.owner;
}

////////////////////////////////////////////////////////////////////////////////

void GameUnit::setLiving(bool isLiving)
{
	m_gameUnitData.isLiving = isLiving;
}

////////////////////////////////////////////////////////////////////////////////

bool GameUnit::isLiving() const
{
	return m_gameUnitData.isLiving;
}

////////////////////////////////////////////////////////////////////////////////

void GameUnit::setHunting(bool isHunting)
{
	m_isHunting = isHunting;
}

////////////////////////////////////////////////////////////////////////////////

bool GameUnit::isHunting() const
{
	return m_isHunting;
}

////////////////////////////////////////////////////////////////////////////////

void GameUnit::setHighlighted(bool isHighlighted)
{
	m_gameUnitData.isHighlighted = isHighlighted;
}

////////////////////////////////////////////////////////////////////////////////

bool GameUnit::isHighlighted() const
{
	return m_gameUnitData.isHighlighted;
}

////////////////////////////////////////////////////////////////////////////////

void GameUnit::setArrived(bool hasArrived)
{
	m_gameUnitData.hasArrived = hasArrived;
}

////////////////////////////////////////////////////////////////////////////////

bool GameUnit::hasArrived() const
{
	return m_gameUnitData.hasArrived;
}

////////////////////////////////////////////////////////////////////////////////

void GameUnit::setAcceleration(cv::Vec2f acceleration)
{
	m_acceleration = acceleration;
}

////////////////////////////////////////////////////////////////////////////////

bool GameUnit::collidesWith(const GameUnit &otherGameUnit)
{
	if (isHunting() && otherGameUnit.isHunting())
		return false;

	if (!isHunting() && !otherGameUnit.isHunting())
		return false;

	float distance = norm(position() - otherGameUnit.position());

	if (distance < 2 * s_radius)
		return true;

	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool GameUnit::collidesWith(const GameObstacle &gameObstacle)
{
	float distance = cv::norm(position() - gameObstacle.position());

	if (distance < s_radius + gameObstacle.radius())
		return true;

	return false;
}

////////////////////////////////////////////////////////////////////////////////

void GameUnit::separateFrom(const GameObstacle &gameObstacle)
{
	reflectOn(gameObstacle);

	cv::Point2f direction = position() - gameObstacle.position();
	float distance = cv::norm(direction);

	float overlappingDistance = distance - (s_radius + gameObstacle.radius());

	if (overlappingDistance <= 0)
		return;

	cv::Point2f separatingVector
		= direction * (overlappingDistance * 1.2 / norm(direction));

	float positionX = x() + separatingVector.x;
	float positionY = y() + separatingVector.y;

	setPosition(positionX, positionY);
}

////////////////////////////////////////////////////////////////////////////////

void GameUnit::reflectOn(const GameObstacle &gameObstacle)
{
	// Compute the axes of collision
	cv::Point2f axis1 = position() - gameObstacle.position();

	axis1 = axis1 * (1.0f / norm(axis1));

	// Project the velocities of both objects to the axes in order to obtain the
	// velocity amount in the direction of collision
	float initialSpeedOnAxis1 = m_velocity.dot(axis1);

	// Delete the speed component in the direction of collision. Later, we will
	// add back the new speed component that has changed due to collision
	m_velocity[0] -= axis1.x * initialSpeedOnAxis1;
	m_velocity[1] -= axis1.y * initialSpeedOnAxis1;

	// Compute the speed component after collision
	float finalSpeedOnAxis1;

	finalSpeedOnAxis1 = -initialSpeedOnAxis1;

	// Add as much speed in the direction of collision as we previously computed
	m_velocity[0] += axis1.x * fabs(finalSpeedOnAxis1);
	m_velocity[1] += axis1.y * fabs(finalSpeedOnAxis1);
}

////////////////////////////////////////////////////////////////////////////////

void GameUnit::reflectOnWalls()
{
	if (x() < s_radius / 2)
	{
		x() = s_radius / 2;

		m_velocity[0] = -m_velocity[0];
	}

	if (x() > 480 - s_radius / 2)
	{
		x() = 480 - s_radius / 2;

		m_velocity[0] = -m_velocity[0];
	}

	if (y() < s_radius / 2)
	{
		y() = s_radius / 2;

		m_velocity[1] = -m_velocity[1];
	}

	if (y() > 480 - s_radius / 2)
	{
		y() = 480 - s_radius / 2;

		m_velocity[1] = -m_velocity[1];
	}
}
