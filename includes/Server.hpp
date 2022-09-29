#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <map>
#include <list>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>

#include "KqueueMonitoredFdInfo.hpp"

class Location;
class Connection;

class Server
{
	private:
		unsigned short port;
		std::string ip;
		std::string server_name;
		int socket_fd;
		std::map<std::string, Location> locations;
		std::map<int, Connection> connections;

	public:
		Server();
		Server(const Server &src);
		Server &operator=(const Server &src);
		virtual ~Server();

		unsigned short getPort() const;
		const std::string &getIP() const;
		const std::string &getServerName() const;
		int getSocketFd() const;
		std::map<std::string, Location> &getLocations();
		std::map<int, Connection> &getConnections();

		void setPort(unsigned short port);
		void setIP(const std::string &ip);
		void setServerName(const std::string &server_name);
		void setSocketFd(int socket_fd);

		int createConnection(int server_fd);


};

#endif