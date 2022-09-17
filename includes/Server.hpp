//겟셋, 변수저장, 유효성 및 헬퍼함수

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <map>
#include <list>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>

#include "FdType.hpp"
// #include "Location.hpp"

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
		std::map<int, Connection> connections;//fd, 인스턴스
		// std::map<size_t, std::list<std::string>> session_logs;
		// size_t session_count;

	public:
		Server();
		Server(const Server &src);
		Server &operator=(const Server &src);
		virtual ~Server();

		void setPort(unsigned short port);
		void setIP(const std::string &ip);
		void setServerName(const std::string &server_name);
		void setSocketFd(int socket_fd);

		unsigned short getPort() const;
		const std::string &getIP() const;
		const std::string &getServerName() const;
		int getSocketFd() const;

		std::map<std::string, Location> &getLocations();

		std::map<int, Connection> &getConnections();

		int createConnection(int server_fd);

		// bool isCorrectAuth(Location &location, Connection &connConnection);

};

#endif