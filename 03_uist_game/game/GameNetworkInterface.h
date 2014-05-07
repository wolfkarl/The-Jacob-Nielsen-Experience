#ifndef __GAME_GAMENETWORKINTERFACE_H
#define __GAME_GAMENETWORKINTERFACE_H

#include <boost/asio.hpp>
#include <boost/signals.hpp>
#include <boost/thread/mutex.hpp>

#include "MessageHandler.h"
#include "MessageData.h"

// Forward declarations
class PlayerProfile;

typedef std::vector<MessageHandler*> MessageHandlers;

/**
 * @class GameNetworkInterface
 *
 * @brief Base class for high-level game client and server.
 *
 * This class serves as a base class for the high-level game client and server.
 * It provides a stable interface for both network interfaces.
 */
class GameNetworkInterface
{
	public:
		virtual ~GameNetworkInterface();

		/**
		 * @brief Runs the game network interface.
		 *
		 * Abstract function which runs the game network interface.
		 */
		virtual void run() = 0;

		/**
		 * @brief Stops the game network interface.
		 *
		 * Abstract function which stops the game network interface.
		 */
		virtual void stop() = 0;

		/**
		 * @brief Adds a message handler.
		 *
		 * Adds a message handler reacting to specific incoming messages. Use
		 * the messageType MESSAGE_ALL_TYPES to listen to all types or the
		 * special messageID values MESSAGE_ID_ALL and MESSAGE_ID_EVENT if
		 * necessary.
		 *
		 * @param messageType - The message type to listen to.
		 * @param messageID - A specific message ID to listen to.
		 * @param messageHandlerCallback - The function to call if such a
		 *     message has been received.
		 */
		void addMessageHandler(ContentType messageType, MessageID messageID,
			MessageHandlerCallback messageHandlerCallback);

		/**
		 * @brief Removes all message handlers.
		 *
		 * Removes all message handlers linked to a specific messageID.
		 *
		 * @param messageID - The message ID of which to remove message
		 *     handlers.
		 */
		void removeAllMessageHandlers(MessageID messageID);

		/**
		 * @brief Removes all message handlers.
		 *
		 * Removes absolutely all message handlers from the interface.
		 */
		void removeAllMessageHandlers();

		/**
		 * @brief Abstract method sending a message.
		 *
		 * Abstract method sending a message.
		 *
		 * @param messageData - The message’s data.
		 * @param receiverID - The ID of the receiver.
		 */
		virtual void send(MessageData messageData, PlayerID receiverID) = 0;

	protected:
		/**
		 * @brief Handles incoming messages.
		 *
		 * Handles incoming messages and calls the responsible message callback.
		 *
		 * @param messageData - The message’s data.
		 * @param senderID - The ID of the sender.
		 */
		void handleMessage(MessageData messageData, PlayerID senderID);

	private:
		/** @brief The list of all message handlers. */
		MessageHandlers m_messageHandlers;

		/** @brief Mutex ensuring thread-safety of message handlers. */
		boost::mutex m_messageHandlerMutex;
};

#endif
