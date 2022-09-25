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

#define GENERAL_RESPONSE 0
#define CGI_RESPONSE 1
#define REDIRECT_RESPONSE 2
#define AUTOINDEX_RESPONSE 3
#define LOG_RESPONSE 4

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
// class Kqueue;

class Webserver
{
private:
	static Webserver *webserver_inst;

	std::vector<Server> real_server; // real server
	std::map<int, Server *> servers_fd; // server fd and server pointer
	Kqueue kq;
	std::map<int, KqueueMonitoredFdInfo *> fd_map; // 생성된 소켓fd와 소켓fd식별인스턴스 배열

	bool returnFalseWithMsg(const char *str);
	bool isReserved(const std::string &src);


public:
	Webserver();
	virtual ~Webserver();

	static Webserver *getWebserverInst();

	bool initKqueue();
	bool initServers();
	
	void setFdMap(int fd, KqueueMonitoredFdInfo *FdInstance);
	void clrFDonTable(int fd);

	std::map<int, KqueueMonitoredFdInfo *> &getFdMap();

	bool execEventQueue();
	void execMonitoredEvent(struct kevent *monitor_event);

	Kqueue &getKq();

	std::vector<Server> &getRealServer();

	int isValidRequestwithConfig(Connection &connection);
	
	int isMultipart(Connection &connection, Location &location);
	int isRedirect(Connection &connection, Location &location);
	bool isCgi(Location &location, Request &request);
	int isAutoIndex(Connection &connection, Location &location);
	int defaultToHttpMethod(Connection &connection, Location &location);
	std::string isValidIndexFile(std::string path, Location &location);

	Location &findLocation(Server &server, const std::string &uri);
	bool isCgiRequest(Location &location, Request &request);
	int unlinkFileAndFolder(std::string path, int level);

	int presetResponseAboutHttpMethod(Connection &connection, Location &location);
	int createFileWithSetup(std::string path);
	int isDirectoryName(const std::string &path);

	void disconnect_connection(Connection &connection);
	int sendResponse(Connection &connection, int monitor_event_fd);
	int makePostPutResponse(KqueueMonitoredFdInfo *monitor_fd, int monitor_event_fd);
	int writeOnPipe(KqueueMonitoredFdInfo *monitor_fd, int monitor_event_fd);
	int makeUploadResponse(KqueueMonitoredFdInfo *monitor_fd, int monitor_event_fd);
};

#endif