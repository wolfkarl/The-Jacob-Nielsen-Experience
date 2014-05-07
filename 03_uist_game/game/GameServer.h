#ifndef __GAME_GAMESERVER_H
#define __GAME_GAMESERVER_H

#include <vector>

#include "MessageData.h"
#include "ForwardDeclarations.h"

/**
 * @class GameServer
 *
 * @brief The game serving serving as the game’s backend.
 *
 * The game server serves as the game’s backend. It compute the game logic and
 * sends the clients updates about changes in the game.
 */
class GameServer
{
	public:
		/**
		 * @brief Runs the game server.
		 *
		 * Runs the game server and initializes all of the game server’s
		 * components.
		 */
		void run();

		/**
		 * @brief Stops the game server.
		 *
		 * Stops the game server and all of its components.
		 */
		void stop();

		/**
		 * @brief The game client’s main loop.
		 *
		 * Executes the game client’s main loop which eventually renders the
		 * scene.
		 */
		void loop();

		void loadGame(int levelNumber);
		void startGame();

	protected:
		void initializeMessageHandlers();

		void handleHighlightRequest(MessageData messageData);
		void handleMoveRequest(MessageData messageData);

		void processGame(float timeFactor);

		void handleSessionAccepted(NetworkServerSession *session);
		void handleSessionClosed(NetworkServerSession *session);

		// Components
		GameNetworkServer *m_gameNetworkServer;

		GamePtr m_game;

		PlayerProfiles m_players;
};

#endif
