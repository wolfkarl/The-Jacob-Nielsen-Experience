#ifndef __NETWORK_NETWORKCLIENT_H
#define __NETWORK_NETWORKCLIENT_H

#include <boost/asio.hpp>
#include <boost/signals.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#include <deque>

#include "MessageData.h"

// Forward declarations
class Message;

/**
 * @class NetworkClient
 *
 * @brief A TCP network client.
 *
 * A TCP network client. It sends messages to a server and is able to receive
 * messages as well. It therefore manages TCP connections automatically and runs
 * in its own thread.
 */
class NetworkClient
{
	public:
		NetworkClient();

		/**
		 * @brief Runs the network client.
		 *
		 * Sets up a connection to the specified server and starts listening for
		 * incoming messages.
		 */
		void run();

		/**
		 * @brief Stops the network client.
		 *
		 * Closes the connection carefully, waits for the connection to tear
		 * down and finally ends up the network client thread.
		 */
		void stop(bool failed = false);

		/**
		 * @brief Returns whether the network client is connected to a server or
		 * not.
		 *
		 * Returns true if the network client successfully connected to the
		 * server and thus actively listens for incoming messages. Returns false
		 * if the client never connected, closed the connection, or if an error
		 * occurred.
		 *
		 * @return - True if the network client is active, false otherwise.
		 */
		bool isConnected();

		/**
		 * @brief Returns whether the network client accepts connections or not.
		 *
		 * Returns whether the network client is currently being accepting
		 * server (re-)connections or not.
		 *
		 * @return - True if the network client accepts connections, false
		 *     otherwise.
		 */
		bool acceptsConnection();

		/**
		 * @brief Sends the message to the server.
		 *
		 * Reads the message's data which has to be exchanged via network and
		 * sends it to the server.
		 *
		 * @param messageData - The Message which shall be sent to the server.
		 */
		void send(MessageData messageData);

		/**
		 * @brief Connects to a server.
		 *
		 * Establishes a connection to a specified server.
		 *
		 * @param serverAddress - The address of the server to connect to.
		 */
		void connectToServer(std::string serverAddress);

		/**
		 * @brief Disconnects from a server.
		 *
		 * Terminates the connection to a server and closes it cleanly.
		 */
		void disconnectFromServer();

		/**
		 * @brief Reconnects to a server.
		 *
		 * Establishes a new connection to the same server as before.
		 */
		void reconnectToServer();

		/**
		 * @brief A signal called when the connection to the server establishes
		 *     successfully.
		 */
		boost::signal<void ()> onConnectionEstablished;

		/**
		 * @brief A signal called when the connection to the server has been
		 *     closed.
		 */
		boost::signal<void ()> onConnectionClosed;

		/**
		 * @brief A signal called when connecting to the server has failed.
		 */
		boost::signal<void ()> onConnectionFailed;

		/**
		 * @brief A signal called each time the network client receives a
		 * complete message from the server.
		 */
		boost::signal<void (MessageData)> onMessageReceived;

	protected:
		/**
		 * @brief Sets up the network client for a server connection.
		 *
		 * Sets up everything necessary to establish a connection.
		 */
		void load();

		/**
		 * @brief Tears down the connection.
		 *
		 * Closes the connection and stops the network service.
		 */
		void unload(bool failed = false);

		/**
		 * @brief Starts accepting server messages.
		 *
		 * Waits for the connection to establish and starts accepting messages
		 * from the server.
		 */
		void startAccept();

		/**
		 * @brief Callback for newly established connections.
		 *
		 * Reacts on newly established connections. Starts reading messages from
		 * the server.
		 *
		 * @param error - An error code returned by Boost.
		 */
		void handleConnect(const boost::system::error_code &error);

		/**
		 * @brief Callback for completely received message headers.
		 *
		 * Reads the header of the currently incoming message. Starts waiting
		 * for the message's content.
		 *
		 * @param error - An error code returned by Boost.
		 */
		void handleReadHeader(const boost::system::error_code &error);

		/**
		 * @brief Callback for completely received message bodies.
		 *
		 * Reads the body of the currently incoming message. Starts waiting for
		 * the next message.
		 *
		 * @param error - An error code returned by Boost.
		 */
		void handleReadBody(const boost::system::error_code &error);

		/**
		 * @brief Sends a message to the server in the network client's thread.
		 *
		 * Contrary to send, this method sends the message in the network
		 * client's thread instead of the caller's. Puts the message into the
		 * network service's queue in order to send it soon.
		 *
		 * @param messageData - Data of a Message which shall be sent.
		 */
		void deliver(MessageData messageData);

		/**
		 * @brief Callback used whenever a message has been transmitted.
		 *
		 * Removes the message from the sending queue and starts transmitting
		 * the next message if one remains.
		 */
		void handleWrite(const boost::system::error_code &error);

		/** @brief Network service. */
		boost::shared_ptr<boost::asio::io_service> m_ioService;

		/** @brief Network socket. */
		boost::shared_ptr<boost::asio::ip::tcp::socket> m_socket;

		/** @brief Iterates through the host’s address list. */
		boost::asio::ip::tcp::resolver::iterator m_iterator;

		/** @brief Buffer for incoming messages. */
		char m_readBuffer[MAX_MESSAGE_LENGTH];

		/** @brief Currently incoming, partly composed message. */
		MessageData m_currentMessage;

		/** @brief Queue of outgoing messages. */
		std::deque<MessageData> m_writeMessageQueue;

		/** @brief Mutex ensuring thread-safety of the message queue. */
		boost::mutex m_writeMessageQueueMutex;

		/** @brief Indicator whether the network client is connected or not. */
		bool m_isConnected;

		/** @brief Mutex ensuring correct connection information. */
		boost::mutex m_isConnectedMutex;

		/** @brief The address of the currently connected server. */
		std::string m_serverAddress;

		/** @brief Condition variable to wait for a connection to establish. */
		boost::condition_variable m_waitForConnectionCondition;

		/** @brief Mutex ensuring thread-safety of network connections. */
		boost::mutex m_waitForConnectionMutex;
};

#endif
