#ifndef __NETWORK_MESSAGE_H
#define __NETWORK_MESSAGE_H

#include <boost/asio.hpp>
#include <boost/signal.hpp>
#include <boost/thread/mutex.hpp>

#include "GameNetworkInterface.h"
#include "MessageData.h"

// Forward declarations
class NetworkServerSession;

typedef unsigned char UpdateFrequency;

/** @brief Update frequencies for network message synchronization. */
enum
{
	UPDATE_FREQUENCY_ONCE = 0,
	UPDATE_FREQUENCY_ON_DEMAND,
	UPDATE_FREQUENCY_ON_CHANGE,
	UPDATE_FREQUENCY_ALWAYS
};

typedef std::vector<PlayerID> PlayerIDs;

/**
 * @class Message
 *
 * @brief A message that can be synchronized via network.
 *
 * Messages can be synchronized via a TCP network connection. Therefore,
 * different members can be registered for network usage. For each member one
 * can define the frequency with which the message is synchronized.
 */
class Message
{
	public:
		/**
		 * @brief Creates a new message.
		 *
		 * Creates a new message. Synchronizable members will be most likely
		 * defined in subclass constructors.
		 *
		 * @param gameNetworkInterface - The network interface to synchronize the
		 *     message with.
		 */
		Message(GameNetworkInterface *gameNetworkInterface);
		virtual ~Message();

		/**
		 * @brief Generates a new, unique message ID.
		 *
		 * Generates a new, unique ID for this message with which the message
		 * will be able to receive updates automatically.
		 */
		void generateMessageID();

		/**
		 * @brief Retrieve the message ID.
		 *
		 * Returns the message’s unique ID if it has one (or, e. g. ID_EVENT).
		 *
		 * @return The message’s ID.
		 */
		MessageID messageID();

		/**
		 * @brief Updates the network data of the message.
		 *
		 * Reads message data being received via network and saves the changes
		 * into the registered member variable.
		 *
		 * @param messageData - The message data package containing the data to
		 *     save.
		 */
		void updateData(MessageData messageData);

		/**
		 * @brief Copy network data directly into this message.
		 *
		 * Reads message data being received via network and saves the whole
		 * content into the registered member variable. In contrast to
		 * updateData, createFromData adopts the message’s ID initially.
		 *
		 * @param messageData - The message data package to initialize the
		 *     message with.
		 */
		void createFromData(MessageData messageData);

		/**
		 * @brief Sends the message to a specified receiver.
		 *
		 * Creates a TCP endpoint to which clients can connect and starts
		 * listening for new connections.
		 *
		 * @param receiverID - The ID of the network participant to send the
		 *     message to.
		 * @param leastFrequency - The least update frequency message data must
		 *     have to be synchronized.
		 */
		void synchronize(PlayerID receiverID,
			UpdateFrequency leastFrequency = UPDATE_FREQUENCY_ONCE);

		/**
		 * @brief Sends the message to a specified receiver.
		 *
		 * Creates a TCP endpoint to which clients can connect and starts
		 * listening for new connections.
		 *
		 * @param receiverIDs - A list of IDs of the network participants to
		 *     send the message to.
		 * @param leastFrequency - The least update frequency message data must
		 *     have to be synchronized.
		 */
		void synchronize(PlayerIDs receiverIDs,
			UpdateFrequency leastFrequency = UPDATE_FREQUENCY_ONCE);

		/**
		 * @brief Sends the message to a specified receiver.
		 *
		 * Creates a TCP endpoint to which clients can connect and starts
		 * listening for new connections.
		 *
		 * @param session - The network server session with which the message
		 *     will be sent.
		 * @param leastFrequency - The least update frequency message data must
		 *     have to be synchronized.
		 */
		void synchronize(NetworkServerSession *session,
			UpdateFrequency leastFrequency = UPDATE_FREQUENCY_ONCE);

		/**
		 * @brief Signal being emitted every time the message receives an
		 *     update.
		 */
		boost::signal<void ()> onUpdate;

	protected:
		/**
		 * @struct RegisteredMessageType
		 *
		 * @brief Information about how a message type has been registered.
		 *
		 * Specifies how a message type has been registered, i. e. which type it
		 * has, where the data to synchronize resides, and with which frequency
		 * the message will be sent via the network connection.
		 */
		struct RegisteredMessageType
		{
			/** @brief Whether sending the data of this type is enabled.*/
			bool isSendingEnabled;

			/** @brief The header matching this type of message data.*/
			MessageHeader header;

			/** @brief Pointer to the data which will be synchronized.*/
			void *data;

			/** @brief The frequency with which the data will be sent.*/
			UpdateFrequency updateFrequency;
		};

		typedef std::vector<RegisteredMessageType> RegisteredMessageTypes;

		/**
		 * @brief Prepares message data for being synchronized via network.
		 *
		 * Specifies a member variable, its type and size to be able to
		 * synchronize it on request via network.
		 *
		 * @param contentType - The data type of the message data.
		 * @param data - Pointer to the data which will be synchronized.
		 * @param contentLength - The size of the data to synchronize.
		 * @param updateFrequency - The frequency with which the message data
		 *     will be transmitted via network.
		 */
		void registerMessageType(ContentType contentType,
			void *data, ContentLength contentLength,
			UpdateFrequency updateFrequency);

		/**
		 * @brief Enables sending message data of a given type.
		 *
		 * Enables sending message data of a given type. It will be synchronized
		 * with the whole message.
		 *
		 * @param contentType - The data type of the message data.
		 */
		void enableSendingMessageType(ContentType contentType);

		/**
		 * @brief Disables sending message data of a given type.
		 *
		 * Disables sending message data of a given type. It will be synchronized
		 * with the whole message.
		 *
		 * @param contentType - The data type of the message data.
		 */
		void disableSendingMessageType(ContentType contentType);

		/**
		 * @brief Sets the message ID directly.
		 *
		 * Specifies the message’s ID used for synchronization manually.
		 *
		 * @param messageID - The new ID the message will have.
		 */
		void setMessageID(MessageID messageID);

		/** @brief The network interface to send and receive updates with. */
		GameNetworkInterface *m_gameNetworkInterface;

	private:
		/** @brief List of all message types that have been registered. */
		RegisteredMessageTypes m_registeredMessageTypes;

		/** @brief Mutex ensuring thread-safe message registration. */
		boost::mutex m_registeredMessageTypesMutex;

		/** @brief The message’s ID. */
		MessageID m_messageID;

		/** @brief The next free, unique message ID. */
		static MessageID s_nextMessageID;
};

#endif
