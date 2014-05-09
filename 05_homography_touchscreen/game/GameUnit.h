#ifndef __GAME_GAME_UNIT_H
#define __GAME_GAME_UNIT_H

#include <boost/shared_ptr.hpp>

#include <opencv2/imgproc/imgproc.hpp>

#include "Message.h"
#include "MessageData.h"
#include "ForwardDeclarations.h"

class GameUnit : public Message
{
	public:
		static float s_maximalVelocity;
		static float s_maximalAcceleration;
		static float s_brakeFactor;
		static float s_radius;

	public:
		GameUnit(GameNetworkInterface *gameNetworkInterface);

		void render(cv::Mat &image);

		void move(float timeDifference);

		void setPosition(float x, float y);
		cv::Point position() const;
		float x() const;
		float &x();
		float y() const;
		float &y();

		void setNumber(uint8_t number);
		uint8_t number();

		void setOwner(PlayerID playerID);
		PlayerID owner() const;

		void setLiving(bool isLiving = true);
		bool isLiving() const;

		void setHunting(bool isHunting = true);
		bool isHunting() const;

		void setArrived(bool hasArrived = true);
		bool hasArrived() const;

		void setHighlighted(bool isHighlighted = true);
		bool isHighlighted() const;

		void setAcceleration(cv::Vec2f acceleration);

		bool collidesWith(const GameUnit &otherGameUnit);
		bool collidesWith(const GameObstacle &gameObstacle);

		void separateFrom(const GameObstacle &gameObstacle);
		void reflectOn(const GameObstacle &gameObstacle);

		void reflectOnWalls();

	protected:
		// Data to synchronize via network
		struct GameUnitData
		{
			float x;
			float y;

			uint8_t number;
			PlayerID owner;

			bool isHighlighted;
			bool isLiving;
			bool hasArrived;
		};

		cv::Vec2f m_velocity;
		cv::Vec2f m_acceleration;

		bool m_isHunting;

		GameUnitData m_gameUnitData;
};

#endif
