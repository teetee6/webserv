#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <list>
#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <sys/stat.h>

#include "KqueueMonitoredFdInfo.hpp"

class Server;
class Location
{
private:
	std::string location_name;
	std::string root;
	std::list<std::string> index;
	std::list<std::string> allow_methods;
	std::map<int, std::string> error_pages;
	int body_limit_size;
	bool auto_index;
	std::map<std::string, std::string> cgi_paths;
	int redirect_return;
	std::string redirect_addr;

public:
	Location();
	Location(const Location &src);
	virtual ~Location();
	Location &operator=(const Location &src);

	void setLocationName(std::string &locaton_name);
	void setRoot(const std::string &root);
	void setBodyLimitSize(int body_limit_size);
	void setAutoIndex(bool auto_index);
	void setCgiPaths(std::map<std::string, std::string> &cgi_paths);
	void setRedirectReturn(int redirect_return);
	void setRedirectAddr(const std::string &redirect_addr);

	const std::string &getLocationName(void);
	const std::string &getRoot();
	std::list<std::string> &getIndex();
	std::list<std::string> &getAllowMethods();
	std::map<int, std::string> &getErrorPages();
	int getBodyLimitSize();
	bool getAutoIndex();
	std::map<std::string, std::string> &getCgiPaths();
	int getRedirectReturn();
	const std::string &getRedirectAddr();
};

#endif