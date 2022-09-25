#ifndef LOCATION_HPP
#define LOCATION_HPP
//겟셋, 변수저장, 유효성 및 헬퍼함수

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
	int request_max_body_size;
	std::string upload_path;
	bool auto_index;
	std::map<std::string, std::string> cgi_infos;
	std::string auth_key;
	int redirect_return;
	std::string redirect_addr;

public:
	Location();
	Location(const Location &src);
	virtual ~Location();
	Location &operator=(const Location &src);

	void setLocationName(std::string &locaton_name);
	void setRoot(const std::string &root);
	// set index;
	// set allow_methods
	// set error_page
	void setRequestMaxBodySize(int request_max_body_size);
	void setUploadPath(const std::string &upload_path);
	void setAutoIndex(bool auto_index);
	void setCgiInfos(std::map<std::string, std::string> &cgi_infos);
	void setAuthKey(const std::string &auth_key);
	void setRedirectReturn(int redirect_return);
	void setRedirectAddr(const std::string &redirect_addr);

	const std::string &getLocationName(void);
	const std::string &getRoot();
	std::list<std::string> &getIndex();
	std::list<std::string> &getAllowMethods();
	std::map<int, std::string> &getErrorPages();
	int getRequestMaxBodySize();
	const std::string &getUploadPath();
	bool getAutoIndex();
	std::map<std::string, std::string> &getCgiInfos();
	const std::string &getAuthKey();
	int getRedirectReturn();
	const std::string &getRedirectAddr();
};

#endif