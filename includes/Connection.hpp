// Connection(request msg)->request
// Connection(response msg)->response

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#define BUFFER_SIZE 65536

#include <iostream>
#include <unistd.h>

#include "FdType.hpp"
#include "Request.hpp"
#include "Response.hpp"

#define DISCONNECT_CONNECTION -1

class Server;

class Connection
{
private:
	//////////////////////////////////////////
	t_status status;
	int server_socket_fd;
	int socket_fd;
	Request request;	 //클라 생성자 생성때 리퀘 생성자 실행
	Response response; // ""
	Server *server;
	//////////////////////////////////////////

public:
	Connection();
	Connection(int server_socket_fd, int socket_fd);
	~Connection();

	void setConnectionFd(int socket_fd);
	void setServerFd(int server_socket_fd);
	void setStatus(t_status status);
	void setServer(Server &server);
	// void setSessionId(size_t session_id);
	// void setSessionFlag(bool flag);

	int getConnectionFd();
	int getServerFd();
	t_status getStatus();
	Server *getServer();

	Request &getRequest();
	Response &getResponse();
	// size_t getSessionId();
	// bool getSessionFlag();

	int readRequest(void);
	// bool parseSessionId(void);
};

#endif