#ifndef __GAME_GAME_H
#define __GAME_GAME_H

#include <boost/timer.hpp>

#include <opencv2/imgproc/imgproc.hpp>

#include "MessageData.h"
#include "ForwardDeclarations.h"

class Game
{
	public:
		Game(GameNetworkInterface *gameNetworkInterface);

		void load(int levelNumber);

		void render(cv::Mat &image);

		void proceed();

		void synchronize(NetworkServerSession *session);
		void synchronize(PlayerID playerID);

		void start();
		bool hasStarted() const;
		bool hasFinished() const;

		const GameUnitPtr unitByID(MessageID messageID) const;
		const GameUnitPtr unitByIndex(PlayerID playerID, uint8_t index) const;

		const GameUnitPtr unitByIndex(uint8_t index) const;

		const GameObstaclePtr obstacleByID(MessageID messageID) const;

		void moveUnit(int index, float angle, float strength);
		void highlightUnit(int index, bool isHighlighted = true);

	protected:
		void initializeMessageHandlers();

		void handleNewPlayerID(MessageData messageData);

		void handleGameUnit(MessageData messageData);
		void handleGameObstacle(MessageData messageData);

		void reset();

		bool m_hasStarted;
		bool m_hasFinished;

		boost::timer m_timer;
		boost::timer m_durationTimer;

		double m_lastUnitTime;

		GameUnits m_gameUnits;
		GameObstacles m_gameObstacles;

		GameNetworkInterface *m_gameNetworkInterface;

		PlayerID m_ownPlayerID;
};

#endif
