#ifndef __GAME_GAMENETWORKCLIENT_H
#define __GAME_GAMENETWORKCLIENT_H

#include "GameNetworkInterface.h"

// Forward declarations
class NetworkClient;

/**
 * @class GameNetworkClient
 *
 * @brief High-level network client.
 *
 * This class provides high-level access to network functionality.
 */
class GameNetworkClient : public GameNetworkInterface
{
	public:
		/**
		 * @brief Runs the game network client.
		 *
		 * Runs the game network client by intializing the low-level client.
		 */
		void run();

		/**
		 * @brief Stops the game network client.
		 *
		 * Stops the game network client by stopping the low-level client.
		 */
		void stop();

		/**
		 * @brief Sends a message.
		 *
		 * Sends a message to a specific receiver.
		 *
		 * @param messageData - The message’s data.
		 * @param receiverID - The ID of the receiver.
		 */
		void send(MessageData messageData, PlayerID receiverID);

		/**
		 * @brief Whether the client is connected.
		 *
		 * Returns whether the client is currently connected to the server or
		 * not.
		 *
		 * @return Whether the client is connected to the server or not.
		 */
		bool isConnected();

		/**
		 * @brief Whether the client currently accepts a server connection.
		 *
		 * Returns whether the client currently accepts a server connection or
		 * is busy transmitting or disconnecting.
		 *
		 * @return Whether the client is able to connect to the server now.
		 */
		bool acceptsConnection();

		/**
		 * @brief Connects the client to a specific server.
		 *
		 * Establishes a connection to the given server.
		 *
		 * @param serverAddress - The address of the game server.
		 */
		void connectToServer(std::string serverAddress);

		/**
		 * @brief Disconnects the client from a specific server.
		 *
		 * Closes a connection to the given server.
		 */
		void disconnectFromServer();

		/**
		 * @brief Reconnects the client to the former server.
		 *
		 * Retries to connect the client to the game server.
		 */
		void reconnectToServer();

		/** @brief Signal emitted whenever a connection has been established. */
		boost::signal<void ()> onConnectionEstablished;

		/** @brief Signal emitted whenever a connection has been closed. */
		boost::signal<void ()> onConnectionClosed;

		/** @brief Signal emitted when establishing a connection has failed. */
		boost::signal<void ()> onConnectionFailed;

	protected:
		/** @brief The low-level network client. */
		NetworkClient *m_networkClient;
};

#endif
