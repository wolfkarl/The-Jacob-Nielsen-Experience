#ifndef __GAME_GAMECLIENT_H
#define __GAME_GAMECLIENT_H

#include <boost/asio.hpp>
#include <boost/signal.hpp>
#include <boost/thread/mutex.hpp>

#include <opencv2/imgproc/imgproc.hpp>

#include "MessageData.h"
#include "ForwardDeclarations.h"

/**
 * @class GameClient
 *
 * @brief The game client serving as the frontend for players.
 *
 * The game client serving as the frontend for players. It renders the 3D scene
 * and plays sound effects, for example.
 */
class GameClient
{
	public:
		GameClient();

		/**
		 * @brief Runs the game client.
		 *
		 * Runs the game client and initializes all of the game client’s
		 * components.
		 */
		void run();

		/**
		 * @brief Connects to a server.
		 *
		 * Connects to port 8642 of the server specified by its address.
		 */
		void connectToServer(std::string serverAddress);

		/**
		 * @brief Stops the game client.
		 *
		 * Stops the game client and all of its components.
		 */
		void stop();

		/**
		 * @brief Signs out the user.
		 *
		 * Disconnects from the server and signs out the user.
		 */
		void signOut();

		GamePtr game();

		/** @brief Signal emitted if stopping the application is requested. **/
		boost::signal<void ()> onStopApplication;

	protected:
		void handleConnectionClosed();

		void cleanUpLeftGame();

		void sendRequests();

		GamePtr m_game;

		GameNetworkClient *m_gameNetworkClient;
};

#endif
