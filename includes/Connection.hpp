#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#define BUFFER_SIZE 65536

#include <iostream>
#include <unistd.h>

#include "KqueueMonitoredFdInfo.hpp"
#include "Request.hpp"
#include "Response.hpp"

#define DISCONNECT_CONNECTION -1

class Server;

class Connection
{
private:
	t_status status;
	int server_socket_fd;
	int socket_fd;
	Request request;
	Response response;
	Server *server;

public:
	Connection();
	Connection(int server_socket_fd, int socket_fd);
	~Connection();

	void setConnectionFd(int socket_fd);
	void setServerFd(int server_socket_fd);
	void setStatus(t_status status);
	void setServer(Server &server);

	int getConnectionFd();
	int getServerFd();
	t_status getStatus();
	Server *getServer();

	Request &getRequest();
	Response &getResponse();

	int readRequest(void);
};

#endif