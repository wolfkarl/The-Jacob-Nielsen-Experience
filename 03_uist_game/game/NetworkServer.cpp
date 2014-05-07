#include "NetworkServer.h"

#include <boost/bind.hpp>

#include "Logging.h"
#include "NetworkServerSession.h"

////////////////////////////////////////////////////////////////////////////////
//
// NetworkServer
//
////////////////////////////////////////////////////////////////////////////////

NetworkServer::NetworkServer()
{
	m_isRunning = false;
}

////////////////////////////////////////////////////////////////////////////////

void NetworkServer::run()
{
	Logging::info("Network server thread is running.");

	try
	{
		// Set up the connection
		load();

		// Start waiting for a connection to establish
		startAccept();

		// Start the network service
		m_ioService->run();
	}
	catch (std::exception &e)
	{
		Logging::error(e.what());
	}

	Logging::info("Network server thread finished.");
}

////////////////////////////////////////////////////////////////////////////////

void NetworkServer::stop()
{
	m_ioService->post(boost::bind(&NetworkServer::release, this));
}

////////////////////////////////////////////////////////////////////////////////

bool NetworkServer::isRunning()
{
	return m_isRunning;
}

////////////////////////////////////////////////////////////////////////////////

void NetworkServer::load()
{
	// Create a TCP connection point for clients on specified port
	m_ioService = new boost::asio::io_service;
	m_endpoint
		= new boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8642);
	m_acceptor = new boost::asio::ip::tcp::acceptor(*m_ioService, *m_endpoint);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkServer::release()
{
	m_isRunning = false;
	m_ioService->stop();

	delete m_ioService;
	delete m_endpoint;
	delete m_acceptor;

	for (std::vector<NetworkServerSession*>::iterator i = m_sessions.begin();
		i != m_sessions.end(); i++)
	{
		delete *i;
	}
}

////////////////////////////////////////////////////////////////////////////////

void NetworkServer::startAccept()
{
	// Create a new session
	NetworkServerSession *new_session = new NetworkServerSession(*m_ioService);

	// Wait for connection to establish
	m_acceptor->async_accept(
		new_session->socket(),
        boost::bind(&NetworkServer::handleAccept, this, new_session,
			boost::asio::placeholders::error));
}

////////////////////////////////////////////////////////////////////////////////

void NetworkServer::handleAccept(NetworkServerSession *session,
	const boost::system::error_code& error)
{
	// If no error occurred, start the newly created connection and insert it
	// into the network server's session list
	if (!error)
	{
		m_sessions.push_back(session);
		session->start();

		onSessionAccepted(session);
	}
	// Else free the new session's memory
	else
		delete session;

	// Wait for next client to connect
	startAccept();
}
