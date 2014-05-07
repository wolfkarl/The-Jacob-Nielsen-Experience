#ifndef __GAME_MOVEREQUEST_H
#define __GAME_MOVEREQUEST_H

#include "Message.h"
#include "ForwardDeclarations.h"

class MoveRequest : public Message
{
	public:
		MoveRequest(GameNetworkInterface *gameNetworkInterface);

		void setUnitIndex(uint8_t unitIndex);
		uint8_t unitIndex();

		void setAngle(float angle);
		float angle();

		void setStrength(float strength);
		float strength();

	protected:
		struct NetworkData
		{
			uint8_t unitIndex;
			float angle;
			float strength;
		};

		NetworkData m_networkData;
};

#endif
