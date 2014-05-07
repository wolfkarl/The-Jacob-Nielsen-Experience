#include "Logging.h"

#include <iostream>

////////////////////////////////////////////////////////////////////////////////
//
// Logging
//
////////////////////////////////////////////////////////////////////////////////

Logging *Logging::s_instance = NULL;

////////////////////////////////////////////////////////////////////////////////

Logging *Logging::instance()
{
	if (!s_instance)
		s_instance = new Logging;

	return s_instance;
}

////////////////////////////////////////////////////////////////////////////////

void Logging::log(std::string text, int level)
{
	boost::lock_guard<boost::mutex> lock(m_loggingMutex);

	switch (level)
	{
		case Level::LEVEL_ERROR:
			std::cerr << "[Error] " << text << std::endl;
			break;

		case Level::LEVEL_WARNING:
			std::cout << "[Warning] " << text << std::endl;
			break;

		case Level::LEVEL_INFO:
			std::cout << "[Info] " << text << std::endl;
			break;

		case Level::LEVEL_DEBUG:
			std::cout << "[Debug] " << text << std::endl;
			break;

		default:
			std::cout << text << std::endl;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////

void Logging::error(std::string text)
{
	instance()->log(text, Level::LEVEL_ERROR);
}

////////////////////////////////////////////////////////////////////////////////

void Logging::warning(std::string text)
{
	instance()->log(text, Level::LEVEL_WARNING);
}

////////////////////////////////////////////////////////////////////////////////

void Logging::info(std::string text)
{
	instance()->log(text, Level::LEVEL_INFO);
}

////////////////////////////////////////////////////////////////////////////////

void Logging::debug(std::string text)
{
	instance()->log(text, Level::LEVEL_DEBUG);
}
