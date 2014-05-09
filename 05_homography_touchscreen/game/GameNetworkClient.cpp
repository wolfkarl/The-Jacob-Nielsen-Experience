#include "GameNetworkClient.h"

#include <boost/thread.hpp>
#include <boost/signal.hpp>

#include "NetworkClient.h"
#include "MessageTypes.h"
#include "Logging.h"

////////////////////////////////////////////////////////////////////////////////
//
// GameNetworkClient
//
////////////////////////////////////////////////////////////////////////////////

void GameNetworkClient::run()
{
	// Create a new network server
	m_networkClient = new NetworkClient();

	// Handle incoming messages
	m_networkClient->onMessageReceived.connect(
		boost::bind(&GameNetworkClient::handleMessage, this, _1, (PlayerID)ID_SERVER));

	// Start the server in a new thread
	boost::thread clientThread(
		boost::bind(&NetworkClient::run, m_networkClient));

	// Forward signals when the connection has successfully been established
	m_networkClient->onConnectionEstablished.connect(
		boost::bind(boost::ref(onConnectionEstablished)));

	// Forward signals when the connection is properly closed
	m_networkClient->onConnectionClosed.connect(
		boost::bind(boost::ref(onConnectionClosed)));

	// Forward signals when the connection cannot be established
	m_networkClient->onConnectionFailed.connect(
		boost::bind(boost::ref(onConnectionFailed)));
}

////////////////////////////////////////////////////////////////////////////////

void GameNetworkClient::stop()
{
	removeAllMessageHandlers();

	m_networkClient->stop();
}

////////////////////////////////////////////////////////////////////////////////

void GameNetworkClient::send(MessageData messageData, PlayerID receiverID)
{
	if (messageData.contentType() == MESSAGE_UNDEFINED)
	{
		std::stringstream error;
		error << "Client message with ID " << (int)messageData.messageID()
			<< " could not be sent (no type given).";
		Logging::error(error.str());

		return;
	}

	if (messageData.messageID() == MESSAGE_ID_NONE)
	{
		std::stringstream error;
		error << "Client message with type " << (int)messageData.contentType()
			<< " could not be sent (no ID given).";
		Logging::error(error.str());

		return;
	}

	// Only allow sending messages to the server
	if (receiverID == ID_SERVER)
		m_networkClient->send(messageData);
}

////////////////////////////////////////////////////////////////////////////////

bool GameNetworkClient::isConnected()
{
	return m_networkClient->isConnected();
}

////////////////////////////////////////////////////////////////////////////////

bool GameNetworkClient::acceptsConnection()
{
	return m_networkClient->acceptsConnection();
}

////////////////////////////////////////////////////////////////////////////////

void GameNetworkClient::connectToServer(std::string serverAddress)
{
	m_networkClient->connectToServer(serverAddress);
}

////////////////////////////////////////////////////////////////////////////////

void GameNetworkClient::disconnectFromServer()
{
	m_networkClient->disconnectFromServer();
}

////////////////////////////////////////////////////////////////////////////////

void GameNetworkClient::reconnectToServer()
{
	m_networkClient->reconnectToServer();
}
