#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <vector>
#include <map>

class Location;
class Request;

class Cgi
{
private:
	int request_fd[2];
	int response_fd[2];
	pid_t pid;

public:
	Cgi(void);
	virtual ~Cgi(void);

	int *getRequestFD(void) const;
	int *getResponseFD(void) const;

	void cgiPipeFdSet(Request &request, Location &location, std::string &file_name, const std::string &ext_path);
	char **setCgiEnvironment(Request &request, Location &location, std::string &file_path);
};

#endif