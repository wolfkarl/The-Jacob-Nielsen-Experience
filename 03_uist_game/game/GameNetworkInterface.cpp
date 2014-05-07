#include "GameNetworkInterface.h"

#include "Logging.h"

////////////////////////////////////////////////////////////////////////////////
//
// GameNetworkInterface
//
////////////////////////////////////////////////////////////////////////////////

GameNetworkInterface::~GameNetworkInterface()
{
	for (std::vector<MessageHandler*>::iterator i = m_messageHandlers.begin();
		i != m_messageHandlers.end(); i++)
	{
		delete *i;
	}
}

////////////////////////////////////////////////////////////////////////////////

void GameNetworkInterface::addMessageHandler(ContentType messageType,
	MessageID messageID, MessageHandlerCallback messageHandlerCallback)
{
	MessageHandler *messageHandler = new MessageHandler;
	messageHandler->setMessageType(messageType);
	messageHandler->setMessageID(messageID);
	messageHandler->setMessageHandlerCallback(messageHandlerCallback);

	m_messageHandlerMutex.lock();
	m_messageHandlers.push_back(messageHandler);
	m_messageHandlerMutex.unlock();
}

////////////////////////////////////////////////////////////////////////////////

void GameNetworkInterface::removeAllMessageHandlers(MessageID messageID)
{
	boost::lock_guard<boost::mutex> lock(m_messageHandlerMutex);

	for (MessageHandlers::iterator messageHandler = m_messageHandlers.begin();
		messageHandler != m_messageHandlers.end();)
	{
		if ((*messageHandler)->messageID() == messageID)
			messageHandler = m_messageHandlers.erase(messageHandler);
		else
			messageHandler++;
	}
}

////////////////////////////////////////////////////////////////////////////////

void GameNetworkInterface::removeAllMessageHandlers()
{
	boost::lock_guard<boost::mutex> lock(m_messageHandlerMutex);

	m_messageHandlers.clear();
}

////////////////////////////////////////////////////////////////////////////////

void GameNetworkInterface::handleMessage(MessageData messageData,
	PlayerID senderID)
{
	bool handlerFound = false;

	// Copy to prevent multiple thread access and deadlocks
	m_messageHandlerMutex.lock();
	MessageHandlers messageHandlers = m_messageHandlers;
	m_messageHandlerMutex.unlock();

	for (std::vector<MessageHandler*>::iterator i = messageHandlers.begin();
		i != messageHandlers.end(); i++)
	{
		if (!(*i)->handles(messageData.contentType(), messageData.messageID()))
			continue;

		(*i)->handle(messageData, senderID);

		handlerFound = true;
	}

	if (!handlerFound)
	{
		std::stringstream error;
		error << "Message has no handler (type "
			<< (int)(messageData.contentType()) << ", id "
			<< (int)(messageData.messageID()) << ", sent from "
			<< senderID << ").";
		Logging::error(error.str());
	}
}
