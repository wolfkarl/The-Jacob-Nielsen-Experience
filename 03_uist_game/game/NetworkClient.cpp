#include "NetworkClient.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "Message.h"
#include "Logging.h"

////////////////////////////////////////////////////////////////////////////////
//
// NetworkClient
//
////////////////////////////////////////////////////////////////////////////////

NetworkClient::NetworkClient()
{
	boost::lock_guard<boost::mutex> lock(m_isConnectedMutex);
	m_isConnected = false;
}

////////////////////////////////////////////////////////////////////////////////

void NetworkClient::run()
{
	Logging::info("Network client thread is running.");

	while (true)
	{
		boost::unique_lock<boost::mutex> lock(m_waitForConnectionMutex);

		while (m_serverAddress == "")
			m_waitForConnectionCondition.wait(lock);

		try
		{
			// Set up the connection
			load();

			// Start waiting for the connection to establish
			startAccept();

			// Start the network service
			m_ioService->run();

			m_ioService.reset();
		}
		catch (std::exception &e)
		{
			Logging::error(e.what());

			stop(true);
		}

		m_serverAddress = "";
	}

	Logging::info("Network client thread finished.");
}

////////////////////////////////////////////////////////////////////////////////

void NetworkClient::stop(bool failed)
{
	// Unload the connection in the network client's thread

	// If there is an active connection, close it
	if (m_ioService)
		m_ioService->post(boost::bind(&NetworkClient::unload, this, failed));
	// Otherwise, just notify that a new connection can be established
	else
		onConnectionClosed();
}

////////////////////////////////////////////////////////////////////////////////

bool NetworkClient::isConnected()
{
	boost::lock_guard<boost::mutex> lock(m_isConnectedMutex);
	return m_isConnected;
}

////////////////////////////////////////////////////////////////////////////////

bool NetworkClient::acceptsConnection()
{
	return m_ioService != NULL;
}

////////////////////////////////////////////////////////////////////////////////

void NetworkClient::load()
{
	// Create a network service and a socket
	m_ioService = boost::shared_ptr<boost::asio::io_service>(
		new boost::asio::io_service);

	// Connect to the server
	boost::asio::ip::tcp::resolver resolver(*m_ioService);
	boost::asio::ip::tcp::resolver::query query(m_serverAddress, "8642");
	m_iterator = resolver.resolve(query);

	m_socket = boost::shared_ptr<boost::asio::ip::tcp::socket>(
		new boost::asio::ip::tcp::socket(*m_ioService));
}

////////////////////////////////////////////////////////////////////////////////

void NetworkClient::unload(bool failed)
{
	Logging::info("Closing the active connection.");

	// Stop the network service
	if (m_ioService)
		m_ioService->stop();

	boost::system::error_code errorCode;

	// Close the socket carefully
	if (m_socket)
		m_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both,
			errorCode);

	if (errorCode)
	{
		Logging::warning("The network client socket could not be closed:");
		Logging::warning((std::string)"\t" + errorCode.message());
	}

	if (m_socket)
		m_socket->close();

	m_socket.reset();
	m_ioService.reset();

	if (failed)
	{
		stop();
		onConnectionFailed();
	}

	onConnectionClosed();

	m_isConnectedMutex.lock();
	m_isConnected = false;
	m_isConnectedMutex.unlock();
}

////////////////////////////////////////////////////////////////////////////////

void NetworkClient::startAccept()
{
	// Wait for connection to establish and start accepting server messages
	m_socket->async_connect(*m_iterator,
		boost::bind(&NetworkClient::handleConnect, this,
			boost::asio::placeholders::error));
}

////////////////////////////////////////////////////////////////////////////////

void NetworkClient::handleConnect(const boost::system::error_code &error)
{
	if (error)
	{
		Logging::info("Connection to server refused.");

		stop();

		// If server is not started yet or error occurred, try to connect again
		onConnectionFailed();
		return;
	}

	// Wait for first incoming message and read its header
	boost::asio::async_read(
		*m_socket,
		boost::asio::buffer(&m_readBuffer, m_currentMessage.headerLength()),
		boost::bind(&NetworkClient::handleReadHeader, this,
			boost::asio::placeholders::error));

	m_isConnectedMutex.lock();
	m_isConnected = true;
	m_isConnectedMutex.unlock();

	// Inform connected objects about the new connection
	onConnectionEstablished();
}

////////////////////////////////////////////////////////////////////////////////

void NetworkClient::handleReadHeader(const boost::system::error_code &error)
{
	if (error)
	{
		Logging::warning((std::string)"Error occurred while reading message "
			+ "header on client.");

		onConnectionClosed();
		return;
	}

	// Copy message header into currently handled message
	m_currentMessage.copyHeaderFrom(m_readBuffer);

	// Wait for rest of the message and read its content
	boost::asio::async_read(
		*m_socket,
		boost::asio::buffer(&m_readBuffer, m_currentMessage.contentLength()),
		boost::bind(&NetworkClient::handleReadBody, this,
			boost::asio::placeholders::error));
}

////////////////////////////////////////////////////////////////////////////////

void NetworkClient::handleReadBody(const boost::system::error_code &error)
{
	if (error)
	{
		Logging::warning((std::string)"Error occurred while reading message "
			+ "body on client.");

		onConnectionClosed();
		return;
	}

	// Read the message's content
	m_currentMessage.copyContentFrom(m_readBuffer);

	onMessageReceived(m_currentMessage);

	// Wait for next incoming message and read its header
	boost::asio::async_read(
		*m_socket,
		boost::asio::buffer(&m_readBuffer, m_currentMessage.headerLength()),
		boost::bind(&NetworkClient::handleReadHeader, this,
			boost::asio::placeholders::error));
}

////////////////////////////////////////////////////////////////////////////////

void NetworkClient::send(MessageData messageData)
{
	// Wait until the setup is finished
	while (! m_isConnected)
		boost::this_thread::sleep(boost::posix_time::seconds(0));

	// Deliver the message in the network client's thread
	m_ioService->post(boost::bind(&NetworkClient::deliver, this, messageData));
}

////////////////////////////////////////////////////////////////////////////////

void NetworkClient::connectToServer(std::string serverAddress)
{
	m_waitForConnectionMutex.lock();
	m_serverAddress = serverAddress;
	m_waitForConnectionMutex.unlock();

	m_waitForConnectionCondition.notify_one();
}

////////////////////////////////////////////////////////////////////////////////

void NetworkClient::disconnectFromServer()
{
	stop();
}

////////////////////////////////////////////////////////////////////////////////

void NetworkClient::reconnectToServer()
{
	startAccept();
}

////////////////////////////////////////////////////////////////////////////////

void NetworkClient::deliver(MessageData messageData)
{
	boost::lock_guard<boost::mutex> writeMessageQueueLock(m_writeMessageQueueMutex);

	bool isWriteInProgress = !m_writeMessageQueue.empty();

	// Insert requested message into sending queue
	m_writeMessageQueue.push_back(messageData);

	// Wait for the queue to be empty before sending
	boost::lock_guard<boost::mutex> isConnectedLock(m_isConnectedMutex);

	if (!m_isConnected || isWriteInProgress)
		return;

	int dataLength = m_currentMessage.headerLength()
		+ m_writeMessageQueue.front().contentLength();

	// Write message via network eventually
	boost::asio::async_write(
		*m_socket,
		boost::asio::buffer(&m_writeMessageQueue.front(), dataLength),
		boost::bind(&NetworkClient::handleWrite, this,
			boost::asio::placeholders::error));
}

////////////////////////////////////////////////////////////////////////////////

void NetworkClient::handleWrite(const boost::system::error_code &error)
{
	if (error)
	{
		Logging::warning((std::string)"Error occurred while writing message "
			+ "for server.");

		onConnectionClosed();
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
		*m_socket,
		boost::asio::buffer(&m_writeMessageQueue.front(), dataLength),
		boost::bind(&NetworkClient::handleWrite, this,
			boost::asio::placeholders::error));
}
