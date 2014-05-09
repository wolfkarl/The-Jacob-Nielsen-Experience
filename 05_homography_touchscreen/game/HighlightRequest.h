#ifndef __GAME_HIGHLIGHTREQUEST_H
#define __GAME_HIGHLIGHTREQUEST_H

#include "Message.h"
#include "ForwardDeclarations.h"

class HighlightRequest : public Message
{
	public:
		HighlightRequest(GameNetworkInterface *gameNetworkInterface);

		void setUnitIndex(uint8_t unitIndex);
		uint8_t unitIndex();

		void setHighlighted(bool isHighlighted = true);
		bool isHighlighted();

	protected:
		struct NetworkData
		{
			uint8_t unitIndex;
			bool isHighlighted;
		};

		NetworkData m_networkData;
};

#endif
