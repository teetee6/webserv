#include "Server.hpp"
#include "Location.hpp"
#include "Connection.hpp"
#include "Webserver.hpp"

Server::Server() : port(-1), socket_fd(-1) {}

Server::Server(const Server &src)
{
	this->ip = src.ip;
	this->port = src.port;
	this->server_name = src.server_name;
	this->socket_fd = src.socket_fd;
	this->locations = src.locations;
}

Server &Server::operator=(const Server &src)
{
	this->ip = src.ip;
	this->port = src.port;
	this->server_name = src.server_name;
	this->socket_fd = src.socket_fd;
	this->locations = src.locations;
	return (*this);
}

Server::~Server()
{
	return;
}

void Server::setPort(unsigned short port)
{
	this->port = port;
	return;
}

void Server::setIP(const std::string &ip)
{
	this->ip = ip;
	return;
}

void Server::setServerName(const std::string &server_name)
{
	this->server_name = server_name;
	return;
}

void Server::setSocketFd(int socket_fd)
{
	this->socket_fd = socket_fd;
	return;
}

unsigned short Server::getPort() const
{
	return (this->port);
}

const std::string &Server::getIP() const
{
	return (this->ip);
}

const std::string &Server::getServerName() const
{
	return (this->server_name);
}

int Server::getSocketFd() const
{
	return (this->socket_fd);
}

std::map<std::string, Location> &Server::getLocations()
{
	return (this->locations);
}

std::map<int, Connection> &Server::getConnections()
{
	return (this->connections);
}

int Server::createConnection(int server_fd)
{
	std::cout << "\033[32m server connection called \033[0m" << std::endl;
	
	struct sockaddr_in connection_addr;
	socklen_t addr_size = sizeof(connection_addr);
	int connection_fd = accept(server_fd, (struct sockaddr *)&connection_addr, &addr_size);
	if (connection_fd == -1)
	{
		std::cerr << "failed to connect connection" << std::endl;
		return (-1);
	}
	
	// std::cout << "I have to answer to [" << connection_fd << "], and my serverFd was "<< server_fd <<"\n";
	fcntl(connection_fd, F_SETFL, O_NONBLOCK);
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	setsockopt(connection_fd, SOL_SOCKET, SO_RCVTIMEO, (void*)&timeout, sizeof(struct timeval));
	setsockopt(connection_fd, SOL_SOCKET, SO_SNDTIMEO, (void*)&timeout, sizeof(struct timeval));

	this->connections[connection_fd].setServerFd(server_fd);
	this->connections[connection_fd].setConnectionFd(connection_fd);
	this->connections[connection_fd].setStatus(REQUEST_RECEIVING);
	this->connections[connection_fd].setServer(*this);

	KqueueMonitoredFdInfo *connection_fd_instance = new KqueueMonitoredFdInfo(CONNECTION_FDTYPE, &this->connections[connection_fd]);
	Webserver::getWebserverInst()->setFdMap(connection_fd, connection_fd_instance);
	Webserver::getWebserverInst()->getKq().createChangeListEvent(connection_fd, "RW");
	
	std::cout << "connected connection : " << connection_fd << std::endl;
	return (connection_fd);
}
