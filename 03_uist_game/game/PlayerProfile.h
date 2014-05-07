#ifndef __GAME_PLAYER_PROFILE_H
#define __GAME_PLAYER_PROFILE_H

#include "MessageData.h"
#include "ForwardDeclarations.h"

class PlayerProfile
{
	public:
		void setPlayerID(PlayerID playerID);
		PlayerID playerID() const;

		void setSession(NetworkServerSession *session);
		NetworkServerSession *session() const;

	protected:
		PlayerID m_playerID;
		NetworkServerSession *m_session;
};

#endif
