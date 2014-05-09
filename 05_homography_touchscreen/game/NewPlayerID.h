#ifndef __GAME_NEW_PLAYER_ID_H
#define __GAME_NEW_PLAYER_ID_H

#include "Message.h"
#include "ForwardDeclarations.h"

class NewPlayerID : public Message
{
	public:
		NewPlayerID(GameNetworkInterface *gameNetworkInterface);

		void setPlayerID(PlayerID playerID);
		PlayerID playerID();

	protected:
		struct NetworkData
		{
			PlayerID playerID;
		};

		NetworkData m_networkData;
};

#endif
