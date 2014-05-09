#include "Game.h"

#include <boost/bind.hpp>

#include "GameNetworkInterface.h"
#include "GameUnit.h"
#include "GameObstacle.h"
#include "MoveRequest.h"
#include "HighlightRequest.h"
#include "NewPlayerID.h"
#include "Logging.h"

////////////////////////////////////////////////////////////////////////////////
//
// Game
//
////////////////////////////////////////////////////////////////////////////////

Game::Game(GameNetworkInterface *gameNetworkInterface)
{
	m_gameNetworkInterface = gameNetworkInterface;

	m_ownPlayerID = ID_NONE;

	m_hasStarted = false;
	m_hasFinished = false;
	m_lastUnitTime = -1.0f;

	initializeMessageHandlers();
}

////////////////////////////////////////////////////////////////////////////////

void Game::initializeMessageHandlers()
{
	m_gameNetworkInterface->addMessageHandler(
		MESSAGE_GAME_UNIT,
		MESSAGE_ID_ALL,
		boost::bind(&Game::handleGameUnit, this, _1));

	m_gameNetworkInterface->addMessageHandler(
		MESSAGE_GAME_OBSTACLE,
		MESSAGE_ID_ALL,
		boost::bind(&Game::handleGameObstacle, this, _1));

	m_gameNetworkInterface->addMessageHandler(
		MESSAGE_NEW_PLAYER_ID,
		MESSAGE_ID_ALL,
		boost::bind(&Game::handleNewPlayerID, this, _1));
}

////////////////////////////////////////////////////////////////////////////////

void Game::load(int levelNumber)
{
	reset();

	std::vector<cv::Point> unitPositionsPlayer1;
	std::vector<cv::Point> unitPositionsPlayer2;
	std::vector<cv::Point> obstaclePositions;
	std::vector<float> obstacleRadii;

	switch (levelNumber)
	{
		case 2:
			unitPositionsPlayer1.push_back(cv::Point(20, 96));
			unitPositionsPlayer1.push_back(cv::Point(100, 80));
			unitPositionsPlayer1.push_back(cv::Point(180, 64));
			unitPositionsPlayer1.push_back(cv::Point(380, 80));
			unitPositionsPlayer1.push_back(cv::Point(460, 96));
			unitPositionsPlayer2.push_back(cv::Point(20, 360));
			unitPositionsPlayer2.push_back(cv::Point(100, 344));
			unitPositionsPlayer2.push_back(cv::Point(300, 328));
			unitPositionsPlayer2.push_back(cv::Point(380, 344));
			unitPositionsPlayer2.push_back(cv::Point(460, 360));
			obstaclePositions.push_back(cv::Point(240, 240));
			obstacleRadii.push_back(50);
			obstaclePositions.push_back(cv::Point(0, 0));
			obstacleRadii.push_back(50);
			obstaclePositions.push_back(cv::Point(0, 480));
			obstacleRadii.push_back(50);
			obstaclePositions.push_back(cv::Point(480, 0));
			obstacleRadii.push_back(50);
			obstaclePositions.push_back(cv::Point(480, 480));
			obstacleRadii.push_back(50);
			break;

		case 3:
			unitPositionsPlayer1.push_back(cv::Point(20, 96));
			unitPositionsPlayer1.push_back(cv::Point(60, 80));
			unitPositionsPlayer1.push_back(cv::Point(100, 64));
			unitPositionsPlayer1.push_back(cv::Point(420, 80));
			unitPositionsPlayer1.push_back(cv::Point(460, 96));
			unitPositionsPlayer2.push_back(cv::Point(20, 360));
			unitPositionsPlayer2.push_back(cv::Point(60, 344));
			unitPositionsPlayer2.push_back(cv::Point(380, 328));
			unitPositionsPlayer2.push_back(cv::Point(420, 344));
			unitPositionsPlayer2.push_back(cv::Point(460, 360));
			obstaclePositions.push_back(cv::Point(240, 0));
			obstacleRadii.push_back(75);
			obstaclePositions.push_back(cv::Point(240, 480));
			obstacleRadii.push_back(75);
			obstaclePositions.push_back(cv::Point(240, 80));
			obstacleRadii.push_back(60);
			obstaclePositions.push_back(cv::Point(240, 400));
			obstacleRadii.push_back(60);
			obstaclePositions.push_back(cv::Point(0, 240));
			obstacleRadii.push_back(50);
			obstaclePositions.push_back(cv::Point(480, 240));
			obstacleRadii.push_back(50);
			obstaclePositions.push_back(cv::Point(160, 240));
			obstacleRadii.push_back(30);
			obstaclePositions.push_back(cv::Point(320, 240));
			obstacleRadii.push_back(30);
			break;

		case 4:
			unitPositionsPlayer1.push_back(cv::Point(340, 96));
			unitPositionsPlayer1.push_back(cv::Point(370, 80));
			unitPositionsPlayer1.push_back(cv::Point(400, 64));
			unitPositionsPlayer1.push_back(cv::Point(430, 80));
			unitPositionsPlayer1.push_back(cv::Point(460, 96));
			unitPositionsPlayer2.push_back(cv::Point(20, 360));
			unitPositionsPlayer2.push_back(cv::Point(50, 344));
			unitPositionsPlayer2.push_back(cv::Point(80, 328));
			unitPositionsPlayer2.push_back(cv::Point(110, 344));
			unitPositionsPlayer2.push_back(cv::Point(140, 360));
			obstaclePositions.push_back(cv::Point(40, 40));
			obstacleRadii.push_back(100);
			obstaclePositions.push_back(cv::Point(440, 440));
			obstacleRadii.push_back(100);
			obstaclePositions.push_back(cv::Point(240, 40));
			obstacleRadii.push_back(75);
			obstaclePositions.push_back(cv::Point(240, 440));
			obstacleRadii.push_back(75);
			obstaclePositions.push_back(cv::Point(200, 200));
			obstacleRadii.push_back(30);
			obstaclePositions.push_back(cv::Point(280, 280));
			obstacleRadii.push_back(30);
			break;

		default:
			unitPositionsPlayer1.push_back(cv::Point(20, 96));
			unitPositionsPlayer1.push_back(cv::Point(60, 96));
			unitPositionsPlayer1.push_back(cv::Point(100, 96));
			unitPositionsPlayer1.push_back(cv::Point(420, 96));
			unitPositionsPlayer1.push_back(cv::Point(460, 96));
			unitPositionsPlayer2.push_back(cv::Point(20, 360));
			unitPositionsPlayer2.push_back(cv::Point(60, 360));
			unitPositionsPlayer2.push_back(cv::Point(380, 360));
			unitPositionsPlayer2.push_back(cv::Point(420, 360));
			unitPositionsPlayer2.push_back(cv::Point(460, 360));
			obstaclePositions.push_back(cv::Point(44, 192));
			obstacleRadii.push_back(44);
			obstaclePositions.push_back(cv::Point(44 + 88, 192));
			obstacleRadii.push_back(44);
			obstaclePositions.push_back(cv::Point(436, 192));
			obstacleRadii.push_back(44);
			obstaclePositions.push_back(cv::Point(436 - 88, 192));
			obstacleRadii.push_back(44);
			obstaclePositions.push_back(cv::Point(216, 272));
			obstacleRadii.push_back(12);
			obstaclePositions.push_back(cv::Point(264, 272));
			obstacleRadii.push_back(12);
			break;
	}

	int unitNumber = 1;

	for (std::vector<cv::Point>::iterator unitPosition = unitPositionsPlayer1.begin();
		 unitPosition != unitPositionsPlayer1.end(); unitPosition++)
	{
		GameUnitPtr newGameUnit(new GameUnit(m_gameNetworkInterface));
		newGameUnit->generateMessageID();
		newGameUnit->setNumber(unitNumber++);
		newGameUnit->setOwner(ID_FIRST_CLIENT);
		newGameUnit->setPosition((*unitPosition).x, (*unitPosition).y);
		newGameUnit->setHunting(false);
		m_gameUnits.push_back(newGameUnit);
	}

	unitNumber = 1;

	for (std::vector<cv::Point>::iterator unitPosition = unitPositionsPlayer2.begin();
		 unitPosition != unitPositionsPlayer2.end(); unitPosition++)
	{
		GameUnitPtr newGameUnit(new GameUnit(m_gameNetworkInterface));
		newGameUnit->generateMessageID();
		newGameUnit->setNumber(unitNumber++);
		newGameUnit->setOwner(ID_FIRST_CLIENT + 1);
		newGameUnit->setPosition((*unitPosition).x, (*unitPosition).y);
		newGameUnit->setHunting(true);
		m_gameUnits.push_back(newGameUnit);
	}

	for (unsigned int i = 0; i < obstaclePositions.size(); i++)
	{
		GameObstaclePtr newGameObstacle(new GameObstacle(m_gameNetworkInterface));
		newGameObstacle->generateMessageID();
		newGameObstacle->setPosition(obstaclePositions[i].x,
									 obstaclePositions[i].y);
		newGameObstacle->setRadius(obstacleRadii[i]);
		m_gameObstacles.push_back(newGameObstacle);
	}

	std::stringstream info;
	info << "Loaded level " << (int)levelNumber << ".";
	Logging::info(info.str());

	m_timer.restart();
}

////////////////////////////////////////////////////////////////////////////////

void Game::reset()
{
	for (unsigned int i = 0; i < m_gameUnits.size(); i++)
		m_gameUnits[i] = GameUnitPtr();

	m_gameUnits.clear();

	for (unsigned int i = 0; i < m_gameObstacles.size(); i++)
		m_gameObstacles[i] = GameObstaclePtr();

	m_gameObstacles.clear();
}

////////////////////////////////////////////////////////////////////////////////

void Game::render(cv::Mat &image)
{
	cv::rectangle(image, cv::Point(0, 0), cv::Point(480, 480),
				  cv::Scalar(32, 32, 32), CV_FILLED);

	cv::rectangle(image, cv::Point(0, 472), cv::Point(480, 480),
				  cv::Scalar(32, 128, 64), CV_FILLED);

	for (unsigned int i = 0; i < m_gameObstacles.size(); i++)
		m_gameObstacles[i]->render(image);

	for (unsigned int i = 0; i < m_gameUnits.size(); i++)
		m_gameUnits[i]->render(image);
}

////////////////////////////////////////////////////////////////////////////////

void Game::proceed()
{
	float timeDifference = m_timer.elapsed();
	m_timer.restart();

	if (!hasStarted())
		return;

	if (m_durationTimer.elapsed() > 60.0f && !hasFinished())
	{
		m_hasFinished = true;

		std::cout << "[Info] Game finished." << std::endl << std::endl;

		int arrivedUnits = 0;

		for (unsigned int i = 0; i < m_gameUnits.size(); i++)
			if (!m_gameUnits[i]->isHunting() && m_gameUnits[i]->hasArrived()
				&& m_gameUnits[i]->isLiving())
				arrivedUnits++;

		int survivingUnits = 0;

		for (unsigned int i = 0; i < m_gameUnits.size(); i++)
			if (!m_gameUnits[i]->isHunting() && !m_gameUnits[i]->hasArrived()
				&& m_gameUnits[i]->isLiving())
				survivingUnits++;

		std::cout << "[Info] Sheep (blue): " << std::endl
			<< "       " << arrivedUnits << " arrived" << std::endl
			<< "       " << survivingUnits << " survived" << std::endl;

		if (m_lastUnitTime > 0)
			std::cout << "       Last unit arrived at " << m_lastUnitTime
				<< std::endl;

		std::cout << std::endl << "       Score: "
			<< (2 * arrivedUnits + survivingUnits) << std::endl;
	}

	if (hasFinished())
		return;

	for (unsigned int i = 0; i < m_gameUnits.size(); i++)
	{
		m_gameUnits[i]->move(timeDifference);
		m_gameUnits[i]->reflectOnWalls();

		if (!m_gameUnits[i]->hasArrived() && !m_gameUnits[i]->isHunting()
			&& m_gameUnits[i]->y() >= 480 - GameUnit::s_radius)
		{
			m_gameUnits[i]->setArrived(true);

			m_lastUnitTime = m_durationTimer.elapsed();
		}
	}

	for (unsigned int i = 0; i < m_gameUnits.size(); i++)
		for (unsigned int j = 0; j < m_gameObstacles.size(); j++)
		{
			if (!m_gameUnits[i]->collidesWith(*m_gameObstacles[j]))
				continue;

			m_gameUnits[i]->separateFrom(*m_gameObstacles[j]);
		}

	for (unsigned int i = 0; i < m_gameUnits.size(); i++)
		for (unsigned int j = 0; j < m_gameUnits.size(); j++)
		{
			if (i == j)
				continue;

			if (!m_gameUnits[i]->collidesWith(*m_gameUnits[j]))
				continue;

			if (!m_gameUnits[i]->isHunting())
				m_gameUnits[i]->setLiving(false);

			if (!m_gameUnits[j]->isHunting())
				m_gameUnits[j]->setLiving(false);
		}
}

////////////////////////////////////////////////////////////////////////////////

void Game::synchronize(NetworkServerSession *session)
{
	for (unsigned int i = 0; i < m_gameUnits.size(); i++)
		m_gameUnits[i]->synchronize(session);

	for (unsigned int i = 0; i < m_gameObstacles.size(); i++)
		m_gameObstacles[i]->synchronize(session);
}

////////////////////////////////////////////////////////////////////////////////

void Game::synchronize(PlayerID playerID)
{
	for (unsigned int i = 0; i < m_gameUnits.size(); i++)
		m_gameUnits[i]->synchronize(playerID);

	for (unsigned int i = 0; i < m_gameObstacles.size(); i++)
		m_gameObstacles[i]->synchronize(playerID);
}

////////////////////////////////////////////////////////////////////////////////

void Game::start()
{
	m_hasStarted = true;

	m_durationTimer.restart();
}

////////////////////////////////////////////////////////////////////////////////

bool Game::hasStarted() const
{
	return m_hasStarted;
}

////////////////////////////////////////////////////////////////////////////////

bool Game::hasFinished() const
{
	return m_hasFinished;
}

////////////////////////////////////////////////////////////////////////////////

void Game::handleNewPlayerID(MessageData messageData)
{
	NewPlayerID newPlayerID(NULL);
	newPlayerID.createFromData(messageData);

	std::cout << "[Info] New player ID received." << std::endl;

	m_ownPlayerID = newPlayerID.playerID();
}

////////////////////////////////////////////////////////////////////////////////

void Game::handleGameUnit(MessageData messageData)
{
	GameUnitPtr matchingGameUnit = unitByID(messageData.messageID());

	if (matchingGameUnit)
		return;

	GameUnitPtr newGameUnit(new GameUnit(m_gameNetworkInterface));
	newGameUnit->createFromData(messageData);
	m_gameUnits.push_back(newGameUnit);
}

////////////////////////////////////////////////////////////////////////////////

void Game::handleGameObstacle(MessageData messageData)
{
	GameObstaclePtr matchingGameObstacle = obstacleByID(messageData.messageID());

	if (matchingGameObstacle)
		return;

	GameObstaclePtr newGameObstacle(new GameObstacle(m_gameNetworkInterface));
	newGameObstacle->createFromData(messageData);
	m_gameObstacles.push_back(newGameObstacle);
}

////////////////////////////////////////////////////////////////////////////////

const GameUnitPtr Game::unitByID(MessageID messageID) const
{
	for (GameUnits::const_iterator i = m_gameUnits.begin();
		 i != m_gameUnits.end(); i++)
	{
		if ((*i)->messageID() == messageID)
			return (*i);
	}

	return GameUnitPtr();
}

////////////////////////////////////////////////////////////////////////////////

const GameUnitPtr Game::unitByIndex(PlayerID playerID, uint8_t index) const
{
	// Return empty unit if accessing opponent's units
	if (m_ownPlayerID != ID_NONE && m_ownPlayerID != playerID)
		return GameUnitPtr();

	uint8_t currentIndex = 0;

	for (GameUnits::const_iterator i = m_gameUnits.begin();
		 i != m_gameUnits.end(); i++)
	{
		if ((*i)->owner() != playerID)
			continue;

		if (currentIndex == index)
			return *i;

		currentIndex++;
	}

	return GameUnitPtr();
}

////////////////////////////////////////////////////////////////////////////////

const GameUnitPtr Game::unitByIndex(uint8_t index) const
{
	return unitByIndex(m_ownPlayerID, index);
}

////////////////////////////////////////////////////////////////////////////////

const GameObstaclePtr Game::obstacleByID(MessageID messageID) const
{
	for (GameObstacles::const_iterator i = m_gameObstacles.begin();
		 i != m_gameObstacles.end(); i++)
	{
		if ((*i)->messageID() == messageID)
			return (*i);
	}

	return GameObstaclePtr();
}

////////////////////////////////////////////////////////////////////////////////

void Game::moveUnit(int index, float angle, float strength)
{
	MoveRequest moveRequest(m_gameNetworkInterface);
	moveRequest.setUnitIndex(index);
	moveRequest.setAngle(angle);
	moveRequest.setStrength(strength);
	moveRequest.synchronize(ID_SERVER);
}

////////////////////////////////////////////////////////////////////////////////

void Game::highlightUnit(int index, bool isHighlighted)
{
	HighlightRequest highlightRequest(m_gameNetworkInterface);
	highlightRequest.setUnitIndex(index);
	highlightRequest.setHighlighted(isHighlighted);
	highlightRequest.synchronize(ID_SERVER);
}
