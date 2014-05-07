#ifndef __NETWORK_NETWORKSERVER_H
#define __NETWORK_NETWORKSERVER_H

#include <boost/asio.hpp>
#include <boost/signal.hpp>

#include <vector>

// Forward declarations
class NetworkServerSession;

/**
 * @class NetworkServer
 *
 * @brief A TCP network server.
 *
 * A TCP network server. It accepts connections from clients and manages them.
 * Each client has its proper ServerSession via which the client communicates.
 */
class NetworkServer
{
	public:
		NetworkServer();

		/**
		 * @brief Runs the network server.
		 *
		 * Creates a TCP endpoint to which clients can connect and starts
		 * listening for new connections.
		 */
		void run();

		/**
		 * @brief Stops the network server.
		 *
		 * Closes all sessions carefully, turns off the TCP endpoint and finally
		 * ends up the network server thread.
		 */
		void stop();

		/**
		 * @brief Returns whether the network server is running or not.
		 *
		 * Returns true if the network server successfully created a network
		 * endpoint and thus actively listens for new connections. Returns false
		 * if the server never created an endpoint, closed the connections, or
		 * if an error occurred.
		 *
		 * @return - True if the network server is active, false otherwise.
		 */
		bool isRunning();

		/**
		 * @brief Called when the network server accepts a new connection.
		 *
		 * A signal called whenever the network server accepted a new connection
		 * and created a session.
		 */
		boost::signal<void (NetworkServerSession*)> onSessionAccepted;

	protected:
		/**
		 * @brief Sets up the network server for client connections.
		 *
		 * Sets up everything necessary to accept connections.
		 */
		void load();

		/**
		 * @brief Tears down the network server.
		 *
		 * Closes the network endpoint, all sessions and stops the network
		 * service.
		 */
		void release();

		/**
		 * @brief Starts accepting connections from clients.
		 *
		 * Waits for the connection to establish and starts accepting messages
		 * from the network server.
		 */
		void startAccept();

		/**
		 * @brief Callback when a new connection was accepted.
		 *
		 * Creates a new session for the newly connected client if no error
		 * occurred.
		 */
		void handleAccept(NetworkServerSession *session,
			const boost::system::error_code &error);

		/** @brief Network service. */
		boost::asio::io_service *m_ioService;

		/** @brief Network endpoint for connections. */
		boost::asio::ip::tcp::endpoint *m_endpoint;

		/** @brief Network connection acceptor. */
		boost::asio::ip::tcp::acceptor *m_acceptor;

		/** @brief Indicator whether the client is running or not. */
		bool m_isRunning;

		/** @brief All active sessions from clients. */
		std::vector<NetworkServerSession*> m_sessions;
};

#endif
