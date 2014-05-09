#ifndef __GAME_GAME_OBSTACLE_H
#define __GAME_GAME_OBSTACLE_H

#include <boost/shared_ptr.hpp>

#include <opencv2/imgproc/imgproc.hpp>

#include "Message.h"
#include "MessageData.h"
#include "ForwardDeclarations.h"

class GameObstacle : public Message
{
	public:
		GameObstacle(GameNetworkInterface *gameNetworkInterface);

		void render(cv::Mat &image);

		void setPosition(float x, float y);
		cv::Point position() const;
		float x() const;
		float y() const;

		void setRadius(float radius);
		float radius() const;

	protected:
		void debug();

		// Data to synchronize via network
		struct GameObstacleData
		{
			float x;
			float y;

			float radius;
		};

		GameObstacleData m_gameObstacleData;
};

#endif
