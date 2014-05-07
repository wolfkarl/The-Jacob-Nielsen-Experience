#include "GameNetworkServer.h"

#include <boost/thread.hpp>
#include <boost/signal.hpp>

#include "NetworkServer.h"
#include "NetworkServerSession.h"
#include "PlayerProfile.h"
#include "Logging.h"

////////////////////////////////////////////////////////////////////////////////
//
// GameNetworkServer
//
////////////////////////////////////////////////////////////////////////////////

GameNetworkServer::GameNetworkServer()
{

}

////////////////////////////////////////////////////////////////////////////////

GameNetworkServer::~GameNetworkServer()
{

}

////////////////////////////////////////////////////////////////////////////////

void GameNetworkServer::run()
{
	m_nextPlayerID = ID_FIRST_CLIENT;

	// Create a new network server
	m_networkServer = new NetworkServer();

	// React to newly connected players
	m_networkServer->onSessionAccepted.connect(
		boost::bind(&GameNetworkServer::handleSessionAccepted, this, _1));

	// Start the network server in a new thread
	boost::thread serverThread(
		boost::bind(&NetworkServer::run, m_networkServer));
}

////////////////////////////////////////////////////////////////////////////////

void GameNetworkServer::stop()
{
	removeAllMessageHandlers();

	m_networkServer->stop();
}

////////////////////////////////////////////////////////////////////////////////

void GameNetworkServer::send(MessageData messageData, PlayerID receiverID)
{
	if (messageData.contentType() == MESSAGE_UNDEFINED)
	{
		std::stringstream error;
		error << "Server message with ID " << (int)messageData.messageID()
			<< " could not be sent (no type given).";
		Logging::error(error.str());

		return;
	}

	if (messageData.messageID() == MESSAGE_ID_NONE)
	{
		std::stringstream error;
		error << "Server message with type " << (int)messageData.contentType()
			<< " could not be sent (no ID given).";
		Logging::error(error.str());

		return;
	}

	// Do not send messages to the server itself
	if (receiverID == ID_SERVER)
	{
		Logging::warning("Tried to send a message from the server to itself.");
		return;
	}

	// If defined, send the message to all of the clients
	if (receiverID == ID_ALL_CLIENTS)
		for (PlayerProfiles::iterator i = m_playerProfiles.begin();
			 i != m_playerProfiles.end(); i++)
		{
			(*i)->session()->send(messageData);
		}

	// Else just send the message to the desired receiver
	else
	{
		PlayerProfilePtr playerProfile = playerProfileByID(receiverID);

		if (playerProfile)
		{
			Logging::error("Cannot send message to non-existing player.");
			return;
		}

		NetworkServerSession *session = playerProfile->session();

		if (session)
			session->send(messageData);
		else
			Logging::warning("Trying to send message to unconnected player.");
	}
}

////////////////////////////////////////////////////////////////////////////////

void GameNetworkServer::handleSessionAccepted(NetworkServerSession *session)
{
	Logging::info("Game server accepted connection.");

	PlayerProfilePtr newProfile(new PlayerProfile);
	newProfile->setPlayerID(m_nextPlayerID++);
	newProfile->setSession(session);
	m_playerProfiles.push_back(newProfile);

	// Start handling messages received from this player
	session->onMessageReceived.connect(
		boost::bind(&GameNetworkServer::handleMessageReceived,
			this, session, _1));

	// Notify if a session has been closed
	session->onClosed.connect(
		boost::bind(boost::ref(onSessionClosed), session));

	onSessionAccepted(session);
}

////////////////////////////////////////////////////////////////////////////////

void GameNetworkServer::handleMessageReceived(NetworkServerSession *session,
	MessageData messageData)
{
	if (messageData.contentType() == ID_NONE)
	{
		Logging::warning((std::string)"Incoming message has no type."
			+ "Closing connection to client.");

		onSessionClosed(session);

		return;
	}

	// Determine corresponding player
	PlayerProfilePtr playerProfile = playerProfileBySession(session);

	if (!playerProfile)
		Logging::warning("Incoming message from unconnected player.");

	// Append the session pointer to the message's data
	messageData.setNetworkServerSession(session);

	PlayerID playerID;

	if (!playerProfile)
		playerID = ID_NEW_CLIENT;
	else
		playerID = playerProfile->playerID();

	// Let the matching message object (which registered to read messages of
	// this type and for this message id) update its data
	handleMessage(messageData, playerID);
}

////////////////////////////////////////////////////////////////////////////////

const PlayerProfilePtr GameNetworkServer::playerProfileByID(PlayerID playerID)
	const
{
	for (PlayerProfiles::const_iterator i = m_playerProfiles.begin();
		 i != m_playerProfiles.end(); i++)
	{
		if ((*i)->playerID() == playerID)
			return (*i);
	}

	return PlayerProfilePtr();
}

////////////////////////////////////////////////////////////////////////////////

const PlayerProfilePtr GameNetworkServer::playerProfileBySession(
	NetworkServerSession *session) const
{
	for (PlayerProfiles::const_iterator i = m_playerProfiles.begin();
		 i != m_playerProfiles.end(); i++)
	{
		if ((*i)->session() == session)
			return (*i);
	}

	return PlayerProfilePtr();
}
