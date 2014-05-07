#include "Message.h"

#include <boost/bind.hpp>

#include "NetworkServerSession.h"
#include "Logging.h"

////////////////////////////////////////////////////////////////////////////////
//
// Message
//
////////////////////////////////////////////////////////////////////////////////

// The first message will receive the first allowed ID
MessageID Message::s_nextMessageID = MESSAGE_ID_FIRST;

////////////////////////////////////////////////////////////////////////////////

Message::Message(GameNetworkInterface *gameNetworkInterface)
	: m_gameNetworkInterface(gameNetworkInterface)
{
	// By default, do not assign an ID
	m_messageID = MESSAGE_ID_NONE;
}

////////////////////////////////////////////////////////////////////////////////

Message::~Message()
{
	if (m_gameNetworkInterface && m_messageID >= MESSAGE_ID_FIRST)
		m_gameNetworkInterface->removeAllMessageHandlers(m_messageID);
}

////////////////////////////////////////////////////////////////////////////////

void Message::createFromData(MessageData messageData)
{
	setMessageID(messageData.messageID());

	updateData(messageData);
}

////////////////////////////////////////////////////////////////////////////////

void Message::updateData(MessageData messageData)
{
	try
	{
		boost::lock_guard<boost::mutex> lock(m_registeredMessageTypesMutex);

		// Look for a registered message type which matches the incoming message
		// type
		for (RegisteredMessageTypes::iterator i = m_registeredMessageTypes.begin();
			i != m_registeredMessageTypes.end(); i++)
		{
			RegisteredMessageType test = *i;

			if ((*i).header.contentType == messageData.contentType())
			{
				messageData.copyTo((*i).data);
				break;
			}
		}

		onUpdate();
	}
	catch (std::exception &e)
	{
		Logging::error(e.what());
	}
}

////////////////////////////////////////////////////////////////////////////////

void Message::synchronize(PlayerID receiverID, UpdateFrequency leastFrequency)
{
	boost::lock_guard<boost::mutex> lock(m_registeredMessageTypesMutex);

	if (m_registeredMessageTypes.empty())
	{
		Logging::error("Message data has not been initialized.");
		return;
	}

	if (!m_gameNetworkInterface)
	{
		Logging::error((std::string)"No network interface has been specified to "
			+ "send the message.");
		return;
	}

	// Look for all synchronizable message types which have been registered and
	// send them if their update frequency is high enough
	for (RegisteredMessageTypes::iterator i = m_registeredMessageTypes.begin();
		i != m_registeredMessageTypes.end(); i++)
	{
		// Don't synchronize message types with a frequency lower than requested
		if ((*i).updateFrequency < leastFrequency)
			continue;

		// Don't synchronize message types if sending is disabled
		if (!(*i).isSendingEnabled)
			continue;

		// Prepare the data to be sent
		MessageData messageData;
		messageData.setHeader((*i).header);
		messageData.copyContentFrom((*i).data);
		m_gameNetworkInterface->send(messageData, receiverID);
	}
}

////////////////////////////////////////////////////////////////////////////////

void Message::synchronize(PlayerIDs receiverIDs, UpdateFrequency leastFrequency)
{
	for (PlayerIDs::iterator receiverID = receiverIDs.begin();
		receiverID != receiverIDs.end(); receiverID++)
	{
		synchronize(*receiverID, leastFrequency);
	}
}

////////////////////////////////////////////////////////////////////////////////

void Message::synchronize(NetworkServerSession *session,
	UpdateFrequency leastFrequency)
{
	boost::lock_guard<boost::mutex> lock(m_registeredMessageTypesMutex);

	if (m_registeredMessageTypes.empty())
	{
		Logging::error("Message data has not been initialized.");
		return;
	}

	if (!session)
	{
		Logging::error("Invalid network server session.");
		return;
	}

	// Look for all synchronizable message types which have been registered and
	// send them if their update frequency is high enough
	for (RegisteredMessageTypes::iterator i = m_registeredMessageTypes.begin();
		i != m_registeredMessageTypes.end(); i++)
	{
		// Don't synchronize message types with a frequency lower than requested
		if ((*i).updateFrequency < leastFrequency)
			continue;

		// Don't synchronize message types if sending is disabled
		if (!(*i).isSendingEnabled)
			continue;

		// Prepare the data to be sent
		MessageData messageData;
		messageData.setHeader((*i).header);
		messageData.copyContentFrom((*i).data);
		session->send(messageData);
	}
}

////////////////////////////////////////////////////////////////////////////////

void Message::registerMessageType(ContentType contentType,
	void *data, ContentLength contentLength, UpdateFrequency updateFrequency)
{
	// Create a message header for this message type
	MessageHeader newMessageHeader;
	newMessageHeader.contentType = contentType;
	newMessageHeader.contentLength = contentLength;
	newMessageHeader.messageID = m_messageID;

	// Store the message header, data and update frequency together
	RegisteredMessageType newRegisteredMessageType;
	newRegisteredMessageType.isSendingEnabled = true;
	newRegisteredMessageType.header = newMessageHeader;
	newRegisteredMessageType.data = data;
	newRegisteredMessageType.updateFrequency = updateFrequency;

	boost::lock_guard<boost::mutex> lock(m_registeredMessageTypesMutex);

	m_registeredMessageTypes.push_back(newRegisteredMessageType);
}

////////////////////////////////////////////////////////////////////////////////

void Message::enableSendingMessageType(ContentType contentType)
{
	boost::lock_guard<boost::mutex> lock(m_registeredMessageTypesMutex);

	for (RegisteredMessageTypes::iterator i = m_registeredMessageTypes.begin();
		i != m_registeredMessageTypes.end(); i++)
	{
		if ((*i).header.contentType == contentType)
			(*i).isSendingEnabled = true;
	}
}

////////////////////////////////////////////////////////////////////////////////

void Message::disableSendingMessageType(ContentType contentType)
{
	boost::lock_guard<boost::mutex> lock(m_registeredMessageTypesMutex);

	for (RegisteredMessageTypes::iterator i = m_registeredMessageTypes.begin();
		i != m_registeredMessageTypes.end(); i++)
	{
		if ((*i).header.contentType == contentType)
			(*i).isSendingEnabled = false;
	}
}

////////////////////////////////////////////////////////////////////////////////

void Message::generateMessageID()
{
	// Get a unique ID
	setMessageID(s_nextMessageID++);
}

////////////////////////////////////////////////////////////////////////////////

void Message::setMessageID(MessageID messageID)
{
	m_messageID = messageID;

	if (m_gameNetworkInterface && messageID >= MESSAGE_ID_FIRST)
		m_gameNetworkInterface->addMessageHandler(MESSAGE_ALL_TYPES,
			messageID, boost::bind(&Message::updateData, this, _1));

	boost::lock_guard<boost::mutex> lock(m_registeredMessageTypesMutex);

	for (RegisteredMessageTypes::iterator i = m_registeredMessageTypes.begin();
		i != m_registeredMessageTypes.end(); i++)
	{
		(*i).header.messageID = messageID;
	}
}

////////////////////////////////////////////////////////////////////////////////

MessageID Message::messageID()
{
	return m_messageID;
}
