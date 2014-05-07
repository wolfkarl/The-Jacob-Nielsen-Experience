#include "HighlightRequest.h"

#include <boost/bind.hpp>

#include "MessageTypes.h"

////////////////////////////////////////////////////////////////////////////////
//
// HighlightRequest
//
////////////////////////////////////////////////////////////////////////////////

HighlightRequest::HighlightRequest(GameNetworkInterface *gameNetworkInterface)
	: Message(gameNetworkInterface)
{
	m_networkData.isHighlighted = true;

	// Set up for network transmission via messages
	registerMessageType(MESSAGE_HIGHLIGHT_REQUEST, &m_networkData,
		sizeof(NetworkData), UPDATE_FREQUENCY_ONCE);

	setMessageID(MESSAGE_ID_EVENT);
}

////////////////////////////////////////////////////////////////////////////////

void HighlightRequest::setUnitIndex(uint8_t unitIndex)
{
	m_networkData.unitIndex = unitIndex;
}

////////////////////////////////////////////////////////////////////////////////

uint8_t HighlightRequest::unitIndex()
{
	return m_networkData.unitIndex;
}

////////////////////////////////////////////////////////////////////////////////

void HighlightRequest::setHighlighted(bool isHighlighted)
{
	m_networkData.isHighlighted = isHighlighted;
}

////////////////////////////////////////////////////////////////////////////////

bool HighlightRequest::isHighlighted()
{
	return m_networkData.isHighlighted;
}
