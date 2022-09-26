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
	//////////////////////////////////////////
	std::string start_line;
	int status;
	std::string cgi_raw;

	std::map<std::string, std::string> headers;
	std::string raw_response;
	std::string body;
	Connection *connection;
	size_t res_idx;

	std::map<std::string, std::string> status_code;

	//////////////////////////////////////////

public:
	Response();
	virtual ~Response();

	std::map<std::string, std::string> mime_type;
	std::map<std::string, std::string> &getHeaders(void);
	std::string &getRawResponse(void);
	std::string &getBody(void);
	// Connection *getConnection();
	size_t getResIdx(void);

	void setConnection(Connection *connection);
	void setResIdx(size_t res_idx);
	void setBody(std::string body);

	void initResponse(void);

	void makeRedirectResponse(Location &location);

	void makeErrorResponse(int status, Location *location);
	void makeAutoIndexResponse(std::string &path, const std::string &uri, Location &location);

	void createErrorPage(int status);

	// void generateAllow(Request &request);
	// void generateDate(void);
	// void generateLastModified(Request &request);
	// void generateContentLanguage(void);
	// void generateContentLocation(Request &request);
	// void generateContentLength(void);
	// void generateContentType(Request &request);
	// void generateLocation(Location &loc);
	// void generateRetryAfter(void);
	// void generateServer(void);
	// void generateWWWAuthenticate();


	// void makeStartLine();
	void makeRawResponse(void);

	//  void makeResponseCgiOrGerneral(KqueueMonitoredFdInfo *resource_fd, int fd, Request &request, long to_read);
	//  void makeResponseCgiOrGerneral(KqueueMonitoredFdInfo *fd_in_FdMap, struct kevent *monitor_event);
	//  bool applyCGIResponse(std::string &raw);
	//  void makeCGIResponseHeader(Request &request);
	//  void makeResponseHeader(Request &request);


	//  void makePutResponse(Request &request);
	 /*
		void makeDeleteResponse(Request &request);
		void makeRedirectResponse(Location &location);
	*/
	void makeResponse(std::string method = "");
	int makeCgiResponse(int curr_event_fd, Request &request);
	void makeDeleteResponse();
	int makeGetHeadResponse(int curr_event_fd, Request &request, long content_length);
	int makeErrorFileResponse(int curr_event_fd);
	// void makeResponsePostPut(Request &request);
	void makePostPutResponse();
	void makeMultipartResponse(std::string uploaded = "NOT");

	std::map<std::string, std::string> &getStatusCode();
	// std::map<std::string, std::string> &getMimeType();
	void initStatusCode(void);
	// void initMimeType(void);
};

#endif