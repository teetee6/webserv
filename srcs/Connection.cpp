#include "Connection.hpp"
#include "Server.hpp"

Connection::Connection()
{

	this->server_socket_fd = -1;
	this->socket_fd = -1;
	this->status = REQUEST_RECEIVING;
	this->request.setConnection(this);
	this->response.setConnection(this);
	this->server = NULL;
}

Connection::Connection(int server_socket_fd, int socket_fd) : server_socket_fd(server_socket_fd), socket_fd(socket_fd)
{

	this->status = REQUEST_RECEIVING;
	this->request.setConnection(this);
	this->response.setConnection(this);
	this->server = NULL;
}

Connection::~Connection() {}

void Connection::setStatus(t_status status)
{
	this->status = status;
	return;
}

void Connection::setServerFd(int server_socket_fd)
{
	this->server_socket_fd = server_socket_fd;
	return;
}

void Connection::setConnectionFd(int socket_fd)
{
	this->socket_fd = socket_fd;
	return;
}

void Connection::setServer(Server &server)
{
	this->server = &server;
}

t_status Connection::getStatus()
{
	return (this->status);
}

int Connection::getServerFd()
{
	return (this->server_socket_fd);
}

Request &Connection::getRequest()
{
	return (this->request);
}

Response &Connection::getResponse()
{
	return (this->response);
}

int Connection::getConnectionFd()
{
	return (this->socket_fd);
}

Server *Connection::getServer()
{
	return (this->server);
}

// consider the situation of multiple requests.
int Connection::readRequest(void)
{
	char buf[BUFFER_SIZE];
	int readed;

	readed = read(this->socket_fd, buf, BUFFER_SIZE - 1);
	if (readed <= 0)
	{
		if (readed == 0)
		{
			std::cout << "hello, you didn't reply me so long time... so I'm gonna disconnect! Bye!\n";
			return (DISCONNECT_CONNECTION);
			// return -1238123;
		}
		else
		{
			std::cerr << "Connection read error!" << std::endl;
			return (DISCONNECT_CONNECTION);
		}
	}
	buf[readed] = 0;
	
	std::cout << "\x1b[31m""here----------------------------------------\n";
	std::cout << buf << std::endl;
	std::cout << "----------------------------------------here\n""\x1b[0m";
	this->request.setRawRequest(buf);
	if (this->request.parseRequest() == true)
		this->status = REQUEST_COMPLETE;
	return (1);
}
