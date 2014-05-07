#include "GameClient.h"

#include <boost/bind.hpp>

#include <opencv2/highgui/highgui.hpp>

#include "Game.h"
#include "GameNetworkClient.h"
#include "Logging.h"

////////////////////////////////////////////////////////////////////////////////
//
// GameClient
//
////////////////////////////////////////////////////////////////////////////////

GameClient::GameClient()
{
	// Initialize the game network client
	m_gameNetworkClient = new GameNetworkClient;

	// React if the server connection is lost
	m_gameNetworkClient->onConnectionClosed.connect(
		boost::bind(&GameClient::handleConnectionClosed, this));

	m_game = GamePtr(new Game(m_gameNetworkClient));
}

////////////////////////////////////////////////////////////////////////////////

void GameClient::run()
{
	// Run the network client after having registered all necessary messages
	m_gameNetworkClient->run();
}

////////////////////////////////////////////////////////////////////////////////

void GameClient::connectToServer(std::string serverAddress)
{
	m_gameNetworkClient->connectToServer(serverAddress);
}

////////////////////////////////////////////////////////////////////////////////

void GameClient::stop()
{
	if (m_gameNetworkClient)
	{
		m_gameNetworkClient->onConnectionClosed.disconnect(
			boost::bind(&GameClient::handleConnectionClosed, this));

		m_gameNetworkClient->stop();
		delete m_gameNetworkClient;
		m_gameNetworkClient = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////

void GameClient::handleConnectionClosed()
{
	cleanUpLeftGame();

	Logging::info("Closed connection to server.");
}

////////////////////////////////////////////////////////////////////////////////

void GameClient::signOut()
{
	m_gameNetworkClient->disconnectFromServer();
}

////////////////////////////////////////////////////////////////////////////////

void GameClient::cleanUpLeftGame()
{

}

////////////////////////////////////////////////////////////////////////////////

void GameClient::sendRequests()
{

}

////////////////////////////////////////////////////////////////////////////////

GamePtr GameClient::game()
{
	return m_game;
}
