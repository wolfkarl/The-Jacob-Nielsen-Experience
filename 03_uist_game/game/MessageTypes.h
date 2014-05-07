#ifndef __GAME_MESSAGETYPES_H
#define __GAME_MESSAGETYPES_H

/**
 * @brief List of all reserved message types in order to identify messages with.
 */
enum
{
	MESSAGE_UNDEFINED = 0,
	MESSAGE_ALL_TYPES,

	MESSAGE_GAME_UNIT,
	MESSAGE_MOVE_REQUEST,
	MESSAGE_HIGHLIGHT_REQUEST,
	MESSAGE_GAME_OBSTACLE,

	MESSAGE_NEW_PLAYER_ID
};

#endif
