#ifndef __MESSAGEHANDLER_H
#define __MESSAGEHANDLER_H

#include <boost/function.hpp>

#include "MessageData.h"
#include "MessageTypes.h"

// A callback that handles message updates from the network
typedef boost::function<void (MessageData, PlayerID)> MessageHandlerCallback;

/**
 * @class MessageHandler
 *
 * @brief Handles a specific type of incoming messages.
 *
 * A message handler reacts to specific message types, IDs and executes a given
 * callback if matching messages have been received.
 */
class MessageHandler
{
	public:
		MessageHandler();

		/**
		 * @brief Sets the message type to react to.
		 *
		 * Sets the message type to react to.
		 *
		 * @param messageType - The message type to react to.
		 */
		void setMessageType(ContentType messageType);

		/**
		 * @brief Returns the message type to which the handler reacts to.
		 *
		 * Returns the message type to which the handler reacts to.
		 *
		 * @return The message type the handler reacts to.
		 */
		ContentType messageType();

		/**
		 * @brief Sets the message ID to react to.
		 *
		 * Sets the message ID to react to.
		 *
		 * @param messageID - The message ID to react to.
		 */
		void setMessageID(MessageID messageID);

		/**
		 * @brief Returns the message ID to which the handler reacts to.
		 *
		 * Returns the message ID to which the handler reacts to.
		 *
		 * @return The message ID the handler reacts to.
		 */
		MessageID messageID();

		/**
		 * @brief Sets the callback which will be executed.
		 *
		 * Sets the callback which will be executed if matching messages have
		 * been received.
		 *
		 * @param messageHandlerCallback - The callback which will be executed.
		 */
		void setMessageHandlerCallback(MessageHandlerCallback
			messageHandlerCallback);

		/**
		 * @brief Returns the callback which will be executed.
		 *
		 * Returns the callback which will be executed if matching messages have
		 * been received.
		 *
		 * @return The callback which will be executed.
		 */
		MessageHandlerCallback messageHandlerCallback();

		/**
		 * @brief Whether the handler will handle a given message.
		 *
		 * Returns whether the handler is responsible for handling messages of
		 * this type and ID.
		 *
		 * @param messageType - The message type the handler will react to.
		 * @param messageID - The message ID the handler will react to.
		 *
		 * @return Whether the handler will handle the given message.
		 */
		bool handles(ContentType messageType, MessageID messageID);

		/**
		 * @brief Executes the matching handler.
		 *
		 * Executes the message handler if the message type and player ID match.
		 *
		 * @param messageData - The message data the handler shall react to.
		 * @param playerID - The player ID the handler shall react to.
		 */
		void handle(MessageData messageData, PlayerID playerID);

	protected:
		bool matchesMessageID(MessageID messageID);
		bool matchesContentType(ContentType messageType);

		// The message type the handler serves
		ContentType m_messageType;

		// The ID of the player the handler wants to handle
		MessageID m_messageID;

		// The callback
		MessageHandlerCallback m_messageHandlerCallback;
};

#endif
