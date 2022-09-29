#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <map>
#include <vector>

#define DEFAULT_STATUS 0

class Request;
class Location;
class ResourceFD;
class Connection;
class KqueueMonitoredFdInfo;

class Response
{
private:
	std::string start_line;
	int status;
	std::string cgi_raw;

	std::map<std::string, std::string> headers;
	std::string raw_response;
	std::string body;
	Connection *connection;
	size_t res_idx;

	std::map<std::string, std::string> status_code;

public:
	Response();
	virtual ~Response();

	void initStatusCode(void);
	void initResponse(void);

	std::map<std::string, std::string> &getHeaders(void);
	std::string &getRawResponse(void);
	std::string &getBody(void);
	size_t getResIdx(void);
	std::map<std::string, std::string> &getStatusCode();

	void setConnection(Connection *connection);
	void setResIdx(size_t res_idx);
	void setBody(std::string body);

	void makeResponse(std::string method = "");
	void makeRedirectResponse(Location &location);
	void makeAutoIndexResponse(std::string &path, const std::string &uri, Location &location);
	int makeCgiResponse(int curr_event_fd, Request &request);
	int makeGetHeadResponse(int curr_event_fd, Request &request, long content_length);
	void makePostPutResponse();
	void makeDeleteResponse();
	void makeMultipartResponse(std::string uploaded = "NOT");
	int makeErrorFileResponse(int curr_event_fd);
	void makeErrorResponse(int status, Location *location);
	void createErrorPage(int status);
};

#endif