#ifndef __NETWORK_NETWORKSERVERSESSION_H
#define __NETWORK_NETWORKSERVERSESSION_H

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/signal.hpp>
#include <boost/thread/mutex.hpp>

#include <deque>

#include "MessageData.h"

/**
 * @class NetworkServerSession
 *
 * @brief A network server session.
 *
 * A network server session handles the connection to a connected client.
 */
class NetworkServerSession
{
	public:
		/**
		 * @brief Creates a server session.
		 *
		 * Creates a server session for a given network IO service.
		 *
		 * @param ioService - The boost::asio IO service.
		 */
		NetworkServerSession(boost::asio::io_service &ioService);

		/**
		 * @brief Returns the TCP socket of the connected client.
		 *
		 * The boost::asio TCP socket connected with the client.
		 *
		 * @return The TCP socket of the connected client.
		 */
		boost::asio::ip::tcp::socket &socket();

		/**
		 * @brief Starts the session.
		 *
		 * Starts reading from the network server session.
		 */
		void start();

		/**
		 * @brief Sends a message to the client.
		 *
		 * Sends the given message data via the session to the connected client.
		 *
		 * @param messageData - The data to send to the client.
		 */
		void send(MessageData messageData);

		/**
		 * @brief Signal emitted whenever a message has been received.
		 */
		boost::signal<void (MessageData)> onMessageReceived;

		/**
		 * @brief Signal emitted when the session has been closed.
		 */
		boost::signal<void ()> onClosed;

	protected:
		/**
		 * @brief Callback handling incoming message headers.
		 *
		 * Reads the message header in order to convert it to a valid message
		 * later.
		 *
		 * @param error - (out) Error state occurring while reading.
		 */
		void handleReadHeader(const boost::system::error_code &error);

		/**
		 * @brief Callback handling incoming message bodies.
		 *
		 * Reads the message body in order to convert it to a valid message
		 * later.
		 *
		 * @param error - (out) Error state occurring while reading.
		 */
		void handleReadBody(const boost::system::error_code &error);

		/**
		 * @brief Sends the message asynchroneously.
		 *
		 * Sends the message via the IO service asynchroneously.
		 *
		 * @param messageData - The message data to send.
		 */
		void deliver(MessageData messageData);

		/**
		 * @brief Callback handling outgoing message bodies.
		 *
		 * Writes the message in order to deliver it.
		 *
		 * @param error - (out) Error state occurring while writing.
		 */
		void handleWrite(const boost::system::error_code &error);

		/** @brief The socket to send and receive messages with. */
		boost::asio::ip::tcp::socket m_socket;

		/** @brief The buffer in which incoming messages are stored. */
		char m_readBuffer[MAX_MESSAGE_LENGTH];

		/** @brief Holds the aggregated incoming data. */
		MessageData m_currentMessage;

		/** @brief List of all messages that will be sent soon. */
		std::deque<MessageData> m_writeMessageQueue;

		/** @brief Mutex ensuring thread-safety of message delivery. */
		boost::mutex m_writeMessageQueueMutex;
};

#endif
