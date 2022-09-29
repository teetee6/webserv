#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <map>
#include <set>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstring>
#include <sstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <arpa/inet.h>

#include <sys/stat.h>
#include <dirent.h>
#include <list>

#include <signal.h>

#include "KqueueMonitoredFdInfo.hpp"
#include "Kqueue.hpp"
#include "Cgi.hpp"

# define RED "\x1b[31m"
# define GREEN "\x1b[32m"
# define BLUE "\x1b[34m"
# define YELLOW "\x1b[33m"
# define MAGENTA "\x1b[35m"
# define CYAN "\x1b[36m"
# define RESET "\x1b[0m"

class Manager;
class Server;
class Location;
class Request;

class Webserver
{
private:
	static Webserver *webserver_inst;

	std::vector<Server> real_server; 
	std::map<int, Server *> servers_fd; 
	Kqueue kq;
	std::map<int, KqueueMonitoredFdInfo *> fd_map; 

public:
	Webserver();
	virtual ~Webserver();

	bool initKqueue();
	bool initServers();
	
	Kqueue &getKq();
	std::map<int, KqueueMonitoredFdInfo *> &getFdMap();
	static Webserver *getWebserverInst();
	std::vector<Server> &getRealServer();

	int isValidRequestwithConfig(Connection &connection);
	int isMultipart(Connection &connection, Location &location);
	int isRedirect(Connection &connection, Location &location);
	bool isCgi(Location &location, Request &request);
	bool isCgiRequest(Location &location, Request &request);
	int isAutoIndex(Connection &connection, Location &location);
	int defaultToHttpMethod(Connection &connection, Location &location);
	std::string isValidIndexFile(std::string path, Location &location);

	bool execEventQueue();
	void execMonitoredEvent(struct kevent *monitor_event);
	int sendResponse(Connection &connection, int monitor_event_fd);

	Location &findLocation(Server &server, const std::string &uri);
	int unlinkFileAndFolder(std::string path, int level);

	int createFileWithSetup(std::string path);
	int isDirectoryName(const std::string &path);

	void setFdMap(int fd, KqueueMonitoredFdInfo *FdInstance);
	void unsetFdMap(int fd);
	void disconnect_connection(Connection &connection);
	int makePostPutResponse(KqueueMonitoredFdInfo *monitor_fd, int monitor_event_fd);
	int writeOnPipe(KqueueMonitoredFdInfo *monitor_fd, int monitor_event_fd);
	int makeUploadResponse(KqueueMonitoredFdInfo *monitor_fd, int monitor_event_fd);
};

#endif