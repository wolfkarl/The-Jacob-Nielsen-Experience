#ifndef __GAME_FORWARD_DECLARATIONS_H
#define __GAME_FORWARD_DECLARATIONS_H

#include <vector>
#include <boost/shared_ptr.hpp>

class GameUnit;
typedef boost::shared_ptr<GameUnit> GameUnitPtr;
typedef std::vector<GameUnitPtr> GameUnits;

class PlayerProfile;
typedef boost::shared_ptr<PlayerProfile> PlayerProfilePtr;
typedef std::vector<PlayerProfilePtr> PlayerProfiles;

class Game;
typedef boost::shared_ptr<Game> GamePtr;

class GameObstacle;
typedef boost::shared_ptr<GameObstacle> GameObstaclePtr;
typedef std::vector<GameObstaclePtr> GameObstacles;

class GameNetworkServer;
class GameNetworkClient;
class GameNetworkInterface;

class NetworkServerSession;

#endif
