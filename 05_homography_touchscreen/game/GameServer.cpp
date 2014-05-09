#include "GameServer.h"


#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <math.h>

#include "Game.h"
#include "GameNetworkServer.h"
#include "MoveRequest.h"
#include "HighlightRequest.h"
#include "PlayerProfile.h"
#include "GameUnit.h"
#include "NewPlayerID.h"
#include "Logging.h"

////////////////////////////////////////////////////////////////////////////////
//
// GameServer
//
////////////////////////////////////////////////////////////////////////////////

void GameServer::run()
{
	m_gameNetworkServer = new GameNetworkServer();
	m_gameNetworkServer->run();

	m_gameNetworkServer->onSessionAccepted.connect(
		boost::bind(&GameServer::handleSessionAccepted, this, _1));

	m_gameNetworkServer->onSessionClosed.connect(
		boost::bind(&GameServer::handleSessionClosed, this, _1));

	initializeMessageHandlers();

	// Start the game server in a new thread
	boost::thread gameServerThread(boost::bind(&GameServer::loop, this));
}

////////////////////////////////////////////////////////////////////////////////

void GameServer::initializeMessageHandlers()
{
	m_gameNetworkServer->addMessageHandler(
		MESSAGE_MOVE_REQUEST,
		MESSAGE_ID_EVENT,
		boost::bind(&GameServer::handleMoveRequest, this, _1));

	m_gameNetworkServer->addMessageHandler(
		MESSAGE_HIGHLIGHT_REQUEST,
		MESSAGE_ID_EVENT,
		boost::bind(&GameServer::handleHighlightRequest, this, _1));
}

////////////////////////////////////////////////////////////////////////////////

void GameServer::handleMoveRequest(MessageData messageData)
{
	MoveRequest moveRequest(NULL);
	moveRequest.createFromData(messageData);

	NetworkServerSession *session = messageData.networkServerSession();
	PlayerProfilePtr playerProfile
		= m_gameNetworkServer->playerProfileBySession(session);

	if (!playerProfile)
	{
		Logging::error("Received move request from non-player client.");
		return;
	}

	GameUnitPtr matchingGameUnit;

	if (m_game)
		matchingGameUnit = m_game->unitByIndex(playerProfile->playerID(),
											   moveRequest.unitIndex());

	if (!matchingGameUnit)
	{
		Logging::error("Game unit requested to move does not exist.");
		return;
	}

	float accelerationX = cos(moveRequest.angle());
	float accelerationY = -sin(moveRequest.angle());

	float strength = std::min(1.0f, std::max(0.0f, moveRequest.strength()));

	strength *= GameUnit::s_maximalAcceleration;

	matchingGameUnit->setAcceleration(cv::Vec2f(accelerationX, accelerationY)
									  * strength);
}

////////////////////////////////////////////////////////////////////////////////

void GameServer::handleHighlightRequest(MessageData messageData)
{
	HighlightRequest highlightRequest(NULL);
	highlightRequest.createFromData(messageData);

	NetworkServerSession *session = messageData.networkServerSession();
	PlayerProfilePtr playerProfile
		= m_gameNetworkServer->playerProfileBySession(session);

	if (!playerProfile)
	{
		Logging::error("Received highlight request from non-player client.");
		return;
	}

	GameUnitPtr matchingGameUnit;

	if (m_game)
		matchingGameUnit = m_game->unitByIndex(playerProfile->playerID(),
											   highlightRequest.unitIndex());

	if (!matchingGameUnit)
	{
		Logging::error("Game unit requested to be highlighted does not exist.");
		return;
	}

	matchingGameUnit->setHighlighted(highlightRequest.isHighlighted());
}

////////////////////////////////////////////////////////////////////////////////

void GameServer::stop()
{
	m_gameNetworkServer->stop();
	delete m_gameNetworkServer;
}

////////////////////////////////////////////////////////////////////////////////

void GameServer::loop()
{
	while (true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(20));

		if (m_game)
			m_game->proceed();

		m_game->synchronize(ID_ALL_CLIENTS);
	}
}

////////////////////////////////////////////////////////////////////////////////

void GameServer::processGame(float timeFactor)
{
	if (m_game)
		m_game->proceed();
}

////////////////////////////////////////////////////////////////////////////////

void GameServer::handleSessionAccepted(NetworkServerSession *session)
{
	PlayerProfilePtr playerProfile
		= m_gameNetworkServer->playerProfileBySession(session);

	if (!playerProfile)
		return;

	NewPlayerID newPlayerID(m_gameNetworkServer);
	newPlayerID.setPlayerID(playerProfile->playerID());
	newPlayerID.synchronize(session);

	m_game->synchronize(session);
}

////////////////////////////////////////////////////////////////////////////////

void GameServer::handleSessionClosed(NetworkServerSession *session)
{

}

////////////////////////////////////////////////////////////////////////////////

void GameServer::loadGame(int levelNumber)
{
	m_game = GamePtr(new Game(m_gameNetworkServer));
	m_game->load(levelNumber);
}

////////////////////////////////////////////////////////////////////////////////

void GameServer::startGame()
{
	if (m_game)
		m_game->start();
}
