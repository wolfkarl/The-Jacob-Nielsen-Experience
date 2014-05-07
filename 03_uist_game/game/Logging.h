#ifndef __GENERAL_LOGGING_H
#define __GENERAL_LOGGING_H

#include <string>

#include <boost/thread/mutex.hpp>

/**
 * @class Logging
 *
 * @brief Logs debug messages.
 *
 * This class serves for logging messages such as debug information, general
 * information, warnings and errors. The messages can be displayed on the
 * terminal or in a special GUI widget if a game client is running.
 */
class Logging
{
	public:
		/**
		 * @brief Returns the logging singleton.
		 *
		 * Returns the singleton instance of the logging class.
		 *
		 * @return The singleton instance of the logging class.
		 */
		static Logging *instance();

		/**
		 * @struct Level
		 *
		 * @brief The different notification levels.
		 *
		 * Contains all possible notification types (debug, info, warning,
		 * error).
		 */
		struct Level
		{
			enum {LEVEL_ERROR, LEVEL_WARNING, LEVEL_INFO, LEVEL_DEBUG};
		};

	protected:
		/** @brief The singleton instance of the logging class. */
		static Logging *s_instance;

	public:
		/**
		 * @brief Logs a message.
		 *
		 * Logs a message text with a specified level of importance.
		 *
		 * @param text - The text to be logged.
		 * @param level - The level of importance of the message.
		 */
		void log(std::string text, int level);

		/**
		 * @brief Logs an error.
		 *
		 * Logs a message with level ›error‹.
		 *
		 * @param text - The text of the error.
		 */
		static void error(std::string text);

		/**
		 * @brief Logs a warning.
		 *
		 * Logs a message with level ›warning‹.
		 *
		 * @param text - The text of the warning.
		 */
		static void warning(std::string text);

		/**
		 * @brief Logs an information.
		 *
		 * Logs a message with level ›information‹.
		 *
		 * @param text - The text of the information.
		 */
		static void info(std::string text);

		/**
		 * @brief Logs an debug.
		 *
		 * Logs a message with level ›debug‹.
		 *
		 * @param text - The text of the debug information.
		 */
		static void debug(std::string text);

	protected:
		/** @brief Mutex providing thread-safe logging. */
		boost::mutex m_loggingMutex;
};

#endif
