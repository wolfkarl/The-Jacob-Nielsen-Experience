#ifndef __NETWORK_MESSAGEDATA_H
#define __NETWORK_MESSAGEDATA_H

#ifdef _MSC_VER

typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;

#else
#include <inttypes.h>
#endif

#include <cstring>
#include <iostream>
#include <limits.h>

#define MAX_MESSAGE_LENGTH 512

// Forward declarations
class NetworkServerSession;

typedef uint16_t MessageID;
typedef uint8_t ContentType;
typedef uint16_t ContentLength;

/** @brief Reserved message identifiers. */
enum {MESSAGE_ID_NONE = 0, MESSAGE_ID_ALL, MESSAGE_ID_EVENT, MESSAGE_ID_FIRST};

typedef uint16_t PlayerID;

/** @brief Reserved network participant identifiers. */
enum {ID_NONE = 0, ID_SERVER, ID_ALL_CLIENTS, ID_NEW_CLIENT, ID_FIRST_CLIENT, ID_LAST_CLIENT = USHRT_MAX};

/**
 * @struct MessageHeader
 *
 * @brief Header of a message.
 *
 * Each message being sent or received via network has its own header to
 * identify the message.
 */
struct MessageHeader
{
	/** @brief The message ID. */
	MessageID messageID;

	/** @brief The message type. */
	ContentType contentType;

	/** @brief The lenght of the message. */
	ContentLength contentLength;
};

/**
 * @struct MessageData
 *
 * @brief The message’s data.
 *
 * This class represents the network-ready data package which can be sent or
 * received via network.
 */
class MessageData
{
	public:
		MessageData();

		/**
		 * @brief Sets the message ID.
		 *
		 * Sets the message ID of the network package.
		 *
		 * @param messageID - The message ID of the network package.
		 */
		void setMessageID(MessageID messageID);

		/**
		 * @brief Returns the message’s ID.
		 *
		 * Returns the message ID of the network package.
		 *
		 * @return The message ID of the network package.
		 */
		MessageID messageID();

		/**
		 * @brief Sets the message type.
		 *
		 * Sets the message type of the network package.
		 *
		 * @param contentType - The message type of the network package.
		 */
		void setContentType(ContentType contentType);

		/**
		 * @brief Returns the message’s type.
		 *
		 * Returns the message type of the network package.
		 *
		 * @return The message type of the network package.
		 */
		ContentType contentType();

		/**
		 * @brief Sets the message header.
		 *
		 * Sets the message header of the network package.
		 *
		 * @param header - The message header of the network package.
		 */
		void setHeader(MessageHeader header);

		/**
		 * @brief Copies the message header from a raw network buffer.
		 *
		 * Copies the message header from a raw network buffer. This is done to
		 * convert raw data into a message object finally.
		 *
		 * @param source - Pointer to the raw network data to read the header
		 *     from.
		 */
		void copyHeaderFrom(void *source);

		/**
		 * @brief Copies the message content from a raw network buffer.
		 *
		 * Copies the message content from a raw network buffer. This is done to
		 * convert raw data into a message object finally.
		 *
		 * @param source - Pointer to the raw network data to read the content
		 *     from.
		 */
		void copyContentFrom(void *source);

		/**
		 * @brief Copies the message header from a raw network buffer.
		 *
		 * Copies the message header from a raw network buffer. This is done to
		 * convert raw data into a message object finally.
		 *
		 * @param source - Pointer to the raw network data to read the header
		 *     from.
		 * @param length - The length of the content to copy.
		 */
		void copyContentFrom(void *source, ContentLength length);

		/**
		 * @brief Copies the message header from a raw network buffer.
		 *
		 * Copies the message header from a raw network buffer. This is done to
		 * convert raw data into a message object finally.
		 *
		 * @param source - The buffer string to copy the content from.
		 */
		void copyContentFromString(char *source);

		/**
		 * @brief Returns the raw message content.
		 *
		 * Returns the raw content of the message, as received from the network.
		 *
		 * @return The raw content of the message.
		 */
		char *content();

		/**
		 * @brief Sets the length of the message content.
		 *
		 * Sets the length of the network package’s content.
		 *
		 * @param contentLength - The length of the message content.
		 */
		void setContentLength(ContentLength contentLength);

		/**
		 * @brief Returns the message’s content length.
		 *
		 * Returns the length of the network package’s content.
		 *
		 * @return The length of the message content.
		 */
		ContentLength contentLength();

		/**
		 * @brief Returns the message’s header length.
		 *
		 * Returns the length of the network package’s header.
		 *
		 * @return The length of the message header.
		 */
		ContentLength headerLength();

		/**
		 * @brief Copies the raw data.
		 *
		 * Copies the raw data, e. g. to convert it into a valid message.
		 *
		 * @param destination - The location to copy the daw message data to.
		 */
		void copyTo(void *destination);

		/**
		 * @brief Sets the network server session.
		 *
		 * Sets the network server session with which the message data is
		 * synchronized.
		 *
		 * @param session - The server session handling the message data.
		 */
		void setNetworkServerSession(NetworkServerSession *session);

		/**
		 * @brief Returns the network server session.
		 *
		 * Returns the network server session with which the message data is
		 * synchronized.
		 *
		 * @return - The server session handling the message data.
		 */
		NetworkServerSession *networkServerSession();

	protected:
		/** @brief The message header of the network package. */
		MessageHeader m_header;

		/** @brief The raw content of the network package. */
		char m_content[MAX_MESSAGE_LENGTH];

		/** @brief The server session handling the message data */
		NetworkServerSession *m_networkServerSession;
};

#endif
