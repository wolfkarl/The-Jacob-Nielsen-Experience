#include "MessageHandler.h"

#include "Logging.h"

////////////////////////////////////////////////////////////////////////////////
//
// MessageHandler
//
////////////////////////////////////////////////////////////////////////////////

MessageHandler::MessageHandler()
{
	m_messageType = MESSAGE_UNDEFINED;
	m_messageID = MESSAGE_ID_NONE;
}

////////////////////////////////////////////////////////////////////////////////

void MessageHandler::setMessageType(ContentType messageType)
{
	m_messageType = messageType;
}

////////////////////////////////////////////////////////////////////////////////

ContentType MessageHandler::messageType()
{
	return m_messageType;
}

////////////////////////////////////////////////////////////////////////////////

void MessageHandler::setMessageID(MessageID messageID)
{
	m_messageID = messageID;
}

////////////////////////////////////////////////////////////////////////////////

MessageID MessageHandler::messageID()
{
	return m_messageID;
}

////////////////////////////////////////////////////////////////////////////////

void MessageHandler::setMessageHandlerCallback(MessageHandlerCallback
	messageHandlerCallback)
{
	m_messageHandlerCallback = messageHandlerCallback;
}

////////////////////////////////////////////////////////////////////////////////

bool MessageHandler::handles(ContentType messageType, MessageID messageID)
{
	return (matchesContentType(messageType) && matchesMessageID(messageID));
}

////////////////////////////////////////////////////////////////////////////////

void MessageHandler::handle(MessageData messageData, PlayerID playerID)
{
	if (!handles(messageData.contentType(), messageData.messageID()))
	{
		Logging::error("Called unsuitable message handler.");
		return;
	}

	if (m_messageHandlerCallback)
		m_messageHandlerCallback(messageData, playerID);
}

////////////////////////////////////////////////////////////////////////////////

bool MessageHandler::matchesMessageID(MessageID messageID)
{
	return (m_messageID == MESSAGE_ID_ALL || messageID == m_messageID);
}

////////////////////////////////////////////////////////////////////////////////

bool MessageHandler::matchesContentType(ContentType messageType)
{
	return (m_messageType == MESSAGE_ALL_TYPES || messageType == m_messageType);
}
