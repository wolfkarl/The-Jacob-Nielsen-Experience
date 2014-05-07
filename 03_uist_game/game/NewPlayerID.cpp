#include "NewPlayerID.h"

#include <boost/bind.hpp>

#include "MessageTypes.h"

////////////////////////////////////////////////////////////////////////////////
//
// NewPlayerID
//
////////////////////////////////////////////////////////////////////////////////

NewPlayerID::NewPlayerID(GameNetworkInterface *gameNetworkInterface)
	: Message(gameNetworkInterface)
{
	m_networkData.playerID = ID_NONE;

	// Set up for network transmission via messages
	registerMessageType(MESSAGE_NEW_PLAYER_ID, &m_networkData,
		sizeof(NetworkData), UPDATE_FREQUENCY_ONCE);

	setMessageID(MESSAGE_ID_EVENT);
}

////////////////////////////////////////////////////////////////////////////////

void NewPlayerID::setPlayerID(PlayerID playerID)
{
	m_networkData.playerID = playerID;
}

////////////////////////////////////////////////////////////////////////////////

PlayerID NewPlayerID::playerID()
{
	return m_networkData.playerID;
}
