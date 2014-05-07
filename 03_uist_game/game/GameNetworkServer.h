#ifndef __GAME_GAMENETWORKSERVER_H
#define __GAME_GAMENETWORKSERVER_H

#include <boost/asio.hpp>
#include <boost/signal.hpp>

#include <vector>

#include "GameNetworkInterface.h"
#include "ForwardDeclarations.h"

// Forward declarations
class Message;
class NetworkServer;
class NetworkServerSession;

/**
 * @class GameNetworkServer
 *
 * @brief High-level network server.
 *
 * This class provides high-level access to network functionality.
 */
class GameNetworkServer : public GameNetworkInterface
{
	public:
		GameNetworkServer();
		~GameNetworkServer();

		/**
		 * @brief Runs the game network server.
		 *
		 * Runs the game network server by intializing the low-level server.
		 */
		void run();

		/**
		 * @brief Stops the game network server.
		 *
		 * Stops the game network server by stopping the low-level server.
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

		const PlayerProfilePtr playerProfileByID(PlayerID playerID) const;
		const PlayerProfilePtr playerProfileBySession(
			NetworkServerSession *session) const;

		/** @brief Signal emitted whenever the session has been closed. */
		boost::signal<void (NetworkServerSession *)> onSessionClosed;

		boost::signal<void (NetworkServerSession *)> onSessionAccepted;

	protected:
		/**
		 * @brief Handles accepted sessions.
		 *
		 * Handles accepted sessions and prepares for listening to messages.
		 *
		 * @param session - The client’s network session.
		 */
		void handleSessionAccepted(NetworkServerSession *session);

		/**
		 * @brief Handles incoming messages.
		 *
		 * Handles incoming messages and calls the responsible message callback.
		 *
		 * @param session - The network session of the sender.
		 * @param messageData - The raw data of the message package.
		 */
		void handleMessageReceived(NetworkServerSession *session,
			MessageData messageData);

		/** @brief The network server which provides low-level functions. */
		NetworkServer *m_networkServer;

		PlayerID m_nextPlayerID;

		PlayerProfiles m_playerProfiles;
};

#endif
