#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <vector>
#include <map>
#include <time.h>

#define MULTIPART 3
#define CHUNKED 2
#define CONTENT_LENGTH 1
#define NOBODY 0

#define PARSING_HEADER 0
#define PARSING_BODY 1

class Connection;
class Webserver;

class Request
{
private:
	//////////////////////////////////////////
	std::string raw_request;

	std::string method;
	std::string uri;
	std::string http_version;
	std::multimap<std::string, std::string> headers;

	std::string raw_header;
	std::string raw_body;

	std::string temp_body;
	int parse_status;
	int body_type;

	std::string path;
	Connection *connection;
	//////////////////////////////////////////
	
	// void parseFirstLine(void);

	void parseHeaders(void);
	void setMethod(std::string &start_line);
	void setUri(std::string &start_line);
	void setHttpVersion(std::string &start_line);

	bool setBodyType(void);
	bool parseBody(void);

public:
	int upload_file;
	Request(void);
	Request(const Request &src);
	virtual ~Request(void){};
	Request &operator=(const Request &src);

	std::string &getRawRequest(void);

	const std::string &getMethod(void) const;
	const std::string &getUri(void) const;
	const std::string &getHttpVersion(void) const;
	std::multimap<std::string, std::string> &getHeaders(void);

	const std::string &getRawHeader(void) const;
	const std::string &getRawBody(void) const;

	const std::string &getTempBody(void) const;

	std::string &getPath(void);
	Connection *getConnection(void);
	int getBodyType(void);

	void setPath(const std::string &path);
	void setConnection(Connection *connection);
	void setRawRequest(char *buf);
	void setRawRequest(std::string str);

	void initRequest(void);

	bool parseRequest(void);
	void parseMultipart(void);
};

#endif
