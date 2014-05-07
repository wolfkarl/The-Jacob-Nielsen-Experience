#include "NetworkServerSession.h"

#include "Logging.h"

////////////////////////////////////////////////////////////////////////////////
//
// NetworkServerSession
//
////////////////////////////////////////////////////////////////////////////////

NetworkServerSession::NetworkServerSession(boost::asio::io_service &ioService)
	: m_socket(ioService)
{
}

////////////////////////////////////////////////////////////////////////////////

boost::asio::ip::tcp::socket &NetworkServerSession::socket()
{
	return m_socket;
}

////////////////////////////////////////////////////////////////////////////////

void NetworkServerSession::start()
{
	Logging::info("Started network session.");

	// Wait for first incoming message and read its header
	boost::asio::async_read(
		m_socket,
		boost::asio::buffer(&m_readBuffer, m_currentMessage.headerLength()),
		boost::bind(&NetworkServerSession::handleReadHeader, this,
			boost::asio::placeholders::error));
}

////////////////////////////////////////////////////////////////////////////////

void NetworkServerSession::handleReadHeader(const boost::system::error_code &error)
{
	if (error)
	{
		Logging::warning((std::string)"Error occurred while reading message "
			+ "header on server. Closing session with client.");

		onClosed();

		return;
	}

	// Read the message's header
	m_currentMessage.copyHeaderFrom(m_readBuffer);

	// Wait for rest of the message and read its content
	boost::asio::async_read(
		m_socket,
		boost::asio::buffer(&m_readBuffer, m_currentMessage.contentLength()),
		boost::bind(&NetworkServerSession::handleReadBody, this,
			boost::asio::placeholders::error));
}

////////////////////////////////////////////////////////////////////////////////

void NetworkServerSession::handleReadBody(const boost::system::error_code &error)
{
	if (error)
	{
		Logging::warning((std::string)"Error occurred while reading message "
			+ "body on server. Closing session with client.");

		onClosed();

		return;
	}

	// Read the message's content
	m_currentMessage.copyContentFrom(m_readBuffer);

	onMessageReceived(m_currentMessage);

	// Wait for next incoming message and read its header
	boost::asio::async_read(
		m_socket,
		boost::asio::buffer(&m_readBuffer, m_currentMessage.headerLength()),
		boost::bind(&NetworkServerSession::handleReadHeader, this,
			boost::asio::placeholders::error));
}

////////////////////////////////////////////////////////////////////////////////

void NetworkServerSession::send(MessageData messageData)
{
	deliver(messageData);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkServerSession::deliver(MessageData messageData)
{
	boost::lock_guard<boost::mutex> lock(m_writeMessageQueueMutex);

	bool isWriteInProgress = !m_writeMessageQueue.empty();

	// Insert requested message into sending queue
	m_writeMessageQueue.push_back(messageData);

	// Wait for the queue to be empty before sending
	if (isWriteInProgress)
		return;

	int dataLength = m_currentMessage.headerLength()
		+ m_writeMessageQueue.front().contentLength();

	// Write message via network eventually
	boost::asio::async_write(
		m_socket,
		boost::asio::buffer(&m_writeMessageQueue.front(), dataLength),
		boost::bind(&NetworkServerSession::handleWrite, this,
			boost::asio::placeholders::error));
}

////////////////////////////////////////////////////////////////////////////////

void NetworkServerSession::handleWrite(const boost::system::error_code &error)
{
	if (error)
	{
		Logging::warning((std::string)"Error occurred while writing message "
			+ "for client. Closing session with client.");

		onClosed();

		return;
	}

	boost::lock_guard<boost::mutex> lock(m_writeMessageQueueMutex);

	// If last message has been sent, remove it from the queue's beginning
	m_writeMessageQueue.pop_front();

	if (m_writeMessageQueue.empty())
		return;

	int dataLength = m_currentMessage.headerLength()
		+ m_writeMessageQueue.front().contentLength();

	// Send next messages if there are remaining ones in the queue
	boost::asio::async_write(
		m_socket,
		boost::asio::buffer(&m_writeMessageQueue.front(), dataLength),
		boost::bind(&NetworkServerSession::handleWrite, this,
			boost::asio::placeholders::error));
}
