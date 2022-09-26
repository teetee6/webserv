#include "Response.hpp"
#include "Request.hpp"
#include "KqueueMonitoredFdInfo.hpp"
#include "Connection.hpp"
#include "Server.hpp"
#include "Webserver.hpp"
#include "Location.hpp"

Response::Response()
{
	this->status = DEFAULT_STATUS;
	this->connection = NULL;
	this->res_idx = 0;
	// this->initMimeType();
	this->initStatusCode();
}

Response::~Response()
{
}

std::map<std::string, std::string> &Response::getHeaders(void)
{
	return (this->headers);
}

std::string &Response::getRawResponse(void)
{
	return (this->raw_response);
}

std::string &Response::getBody(void)
{
	return (this->body);
}

size_t Response::getResIdx(void)
{
	return (this->res_idx);
}

void Response::setConnection(Connection *connection)
{
	this->connection = connection;
}

void Response::setResIdx(size_t res_idx)
{
	this->res_idx = res_idx;
}

void Response::setBody(std::string body)
{
	this->body = body;
}

void Response::initResponse(void)
{
	this->start_line.clear();
	this->headers.clear();
	this->body.clear();
	this->raw_response.clear();
	this->status = DEFAULT_STATUS;
	this->cgi_raw.clear();
	this->res_idx = 0;
}


void Response::createErrorPage(int status)
{
	this->status = status;
	std::stringstream ss;
	std::string str;
	
	ss << this->status;
	ss >> str;

	this->body.clear();
	this->body += std::string("<html>\r\n") + std::string("<head>\r\n") + std::string("<title>") + str + std::string(" ") + std::string(this->getStatusCode().find(str)->second) + std::string("</title>\r\n") + std::string("</head>\r\n") + std::string("<body>\r\n") + std::string("<center>\r\n") + std::string("<h1>") + str + std::string(" ") + this->getStatusCode().find(str)->second + std::string("</h1>\r\n") + std::string("</center>\r\n") + std::string("<hr>\r\n") + std::string("<center>AeronHyosi/1.0</center>\r\n") + std::string("</body>\r\n") + std::string("</html>");
}

void Response::makeRedirectResponse(Location &location)
{ //리다이렉션할때 this->header에 값 집어넣고, 그것들을 포맷바궈써 this->rawresponse애 재할당 메시지 생성
	this->status = location.getRedirectReturn();
	this->headers.insert(std::pair<std::string, std::string>("Location", location.getRedirectAddr()));
	this->makeResponse();
	this->connection->setStatus(RESPONSE_COMPLETE);
}


// parse into this->headers and this->body, and then merge into this->raw_response
void Response::makeErrorResponse(int status, Location *location)
{
	this->status = status;

	// default ErrorPage
	if (location == NULL || location->getErrorPages().count(status) == 0)
	{
		// std::cout << "1 NO" << status << "\n"; 
		this->createErrorPage(status);
		std::stringstream ss;
		std::string str;
		ss << this->body.length();
		ss >> str;
		this->headers.insert(std::pair<std::string, std::string>("Content-Length", str));
		this->makeResponse();
		this->connection->setStatus(RESPONSE_COMPLETE);
		return;
	}
	// specific ErrorPage
	else
	{
		// std::cout << "YES\n";
		int error_file_fd = open(location->getErrorPages()[status].c_str(), O_RDONLY);
		if (error_file_fd == -1)
		{
			makeErrorResponse(500, NULL);
			return;
		}
		KqueueMonitoredFdInfo *error_file_instance = new KqueueMonitoredFdInfo(ERROR_FILE_FDTYPE, this->connection);
		Webserver::getWebserverInst()->setFdMap(error_file_fd, error_file_instance);
		Webserver::getWebserverInst()->getKq().createChangeListEvent(error_file_fd, "R");
	}
}

void Response::makeAutoIndexResponse(std::string &path, const std::string &uri, Location &location)
{
	DIR *dir_ptr;
	struct dirent *file;

	std::stringstream ss;
	std::string str;

	if (path[path.length() - 1] != '/')
		path += '/';

	if ((dir_ptr = opendir(path.c_str())) == NULL)
	{
		this->makeErrorResponse(500, &location);
		return;
	}

	this->body = std::string("<html>\r\n") +std::string("<head>\r\n") +std::string("<title>Index of " + uri + "</title>\r\n") +std::string("<meta charset='UTF-8'>") +std::string("</head>\r\n") +std::string("<body bgcolor=\"white\">\r\n") +std::string("<h1>Index of " + uri + "</h1>\r\n") +std::string("<hr>\r\n") +std::string("<pre>\r\n");

	while ((file = readdir(dir_ptr)) != NULL)
	{
		struct stat sb;
		struct tm *timeinfo;
		char buffer[4096];
		std::string name = std::string(file->d_name);
		if (file->d_type == DT_DIR)
		{
			name += '/';
			this->body += "🗂";
		}
		else
			this->body += "📑";
		this->body += "<a href=\"" + name + "\">" + name + "</a>\r\n";

		if (stat((path + name).c_str(), &sb) == -1)
		{
			this->start_line.clear();
			this->body.clear();
			(void)location;
			this->makeErrorResponse(500, &location);
			return;
		}

		timeinfo = localtime(&sb.st_mtime);
		strftime(buffer, 4096, "%d-%b-%Y %H:%M", timeinfo);
		this->body += "                                        " + std::string(buffer) + "                   ";
		if (S_ISDIR(sb.st_mode))
			this->body += "-\r\n";
		else
		{
			ss << sb.st_size;
			ss >> str;
			this->body += str + "\r\n";
		}
	}
	closedir(dir_ptr);

	
	this->body += std::string("</pre>\r\n") +std::string("<hr>\r\n") +std::string("</body>\r\n") +std::string("</html>");

	this->status = 200;
	this->headers.insert(std::pair<std::string, std::string>("Content-Type", "text/html"));

	this->makeResponse();
	this->connection->setStatus(RESPONSE_COMPLETE);
}

void Response::makeResponse(std::string method)
{
	(void)method;
	// std::cout << "status: " <<  this->status << std::endl;
	/* HEADER */
	if (this->headers.find("Content-Length") == this->headers.end())
	{
		std::stringstream ss;
		std::string str;
		ss << this->body.length();
		ss >> str;
		this->headers.insert(std::pair<std::string, std::string>("Content-Length", str));
	}
	if (this->start_line.find("HTTP") == std::string::npos)
	{
		std::stringstream ss;
		std::string str;
		ss << this->status;
		ss >> str;
		// std::cout << this->status << std::endl;
		// std::cout << str << std::endl;
		std::map<std::string, std::string>::const_iterator iter = this->getStatusCode().find(str);
		
		if (iter == this->getStatusCode().end())
		{
			std::cout << "No Such code\n";
			return;
		}
		this->start_line += "HTTP/1.1 ";
		this->start_line += str;
		this->start_line += " ";
		this->start_line += iter->second;
	}
	// std::cout << "Start Line: " <<  this->start_line << std::endl;
	
	if (this->connection->getRequest().getMethod() == "HEAD")
		this->body.clear();

	this->raw_response += this->start_line;
	this->raw_response += "\r\n";

	for (std::map<std::string, std::string>::const_iterator iter = this->headers.begin(); iter != this->headers.end(); iter++)
	{
		this->raw_response += iter->first;
		this->raw_response += ": ";
		this->raw_response += iter->second;
		this->raw_response += "\r\n";
	}
	// std::cout << "[header to raw_response:]\n" <<  this->raw_response << std::endl;

	this->raw_response += "\r\n";
	this->raw_response += this->body;
	// std::cout << "[the last raw_response:]\n" <<  this->raw_response << std::endl;

	/* BODY */
}
void Response::makePostPutResponse()
{
	this->status = 201;
	this->body = this->connection->getRequest().getRawBody();
	this->makeResponse();
	this->connection->setStatus(RESPONSE_COMPLETE);
}

void Response::makeMultipartResponse(std::string uploaded)
{
	if (uploaded == "UPLOADED")
		this->status = 201;
	else
		this->status = 200;
	this->body = this->connection->getRequest().getRawBody();
	this->makeResponse();
	this->connection->setStatus(RESPONSE_COMPLETE);
}

void Response::makeDeleteResponse()
{ // delete 응답메시지 생성
	this->status = 200;
	this->makeResponse();
	this->connection->setStatus(RESPONSE_COMPLETE);
}

int Response::makeGetHeadResponse(int curr_event_fd, Request &request, long content_length)
{
	// std::cout << "FILE_FDTYPE READ!!!\n";
	// std::cout << "read from [" << curr_event_fd << "]\n";
	// std::cout << content_length << std::endl;
	char buf[BUFFER_SIZE];
	int read_size;

	read_size = read(curr_event_fd, buf, BUFFER_SIZE - 1);
	// std::cout << "Read_size: " << read_size << std::endl;
	if (read_size == -1)
	{
		Webserver::getWebserverInst()->unsetFdMap(curr_event_fd);
		close(curr_event_fd);
		this->makeErrorResponse(500, NULL); // 500 Error
		return 404;
	}
	buf[read_size] = '\0';
	this->body += std::string(buf);
	if (content_length - read_size > 0)
		return 0;
	Webserver::getWebserverInst()->unsetFdMap(curr_event_fd);
	close(curr_event_fd);

	std::cout << "now I deleted the resource fd on fd_map, close the resource [" << curr_event_fd << "]\n";
	for (std::map<int, KqueueMonitoredFdInfo *>::iterator iter = Webserver::getWebserverInst()->getFdMap().begin(); iter != Webserver::getWebserverInst()->getFdMap().end(); ++iter)
	// 	std::cout << iter->first << " AND, " << iter->second << std::endl;
	// std::cout << "is this seen?\n";

	this->status = 200;
	this->makeResponse();
	// std::cout << "is this seen2 ?\n";
	// std::cout << "[raw_response]\n" << this->raw_response << "\n";
	request.getConnection()->setStatus(RESPONSE_COMPLETE);
	return 0;
}

int Response::makeCgiResponse(int curr_event_fd, Request &request)
{
	char buf[BUFFER_SIZE];
	int read_size;

	read_size = read(curr_event_fd, buf, BUFFER_SIZE - 1);
	if (read_size == -1)
	{
		std::cerr << "temporary resource read error!" << std::endl;
		return 404;
	}
	buf[read_size] = '\0';

	this->cgi_raw += buf;
	// std::cout << "buf = " << buf << std::endl;
	if (read_size != 0)
		return 0;
	Webserver::getWebserverInst()->unsetFdMap(curr_event_fd);
	close(curr_event_fd);

	if (cgi_raw.find("X-Powered-By:") != std::string::npos)
	{
		if (cgi_raw.substr(14, 3) == "PHP")
			cgi_raw = cgi_raw.substr(cgi_raw.find("\r\n\r\n") + 4);
	}

	// status-line
	std::vector<std::string> status_line;
	std::size_t status_sep = cgi_raw.find("\r\n");
	// ft_split(cgi_raw.substr(0, status_sep), " ", status_line);
	{
		std::istringstream	iss(cgi_raw.substr(0, status_sep));
		std::string			elem;
		while (iss >> elem)
			status_line.push_back(elem);
	}
	// for(std::vector<std::string>::iterator it = status_line.begin(); it != status_line.end(); it++)
	// {
	// 	std::cout << "{" << *it << "} ";
	// }
	// std::cout << std::endl;
	if (status_line.size() < 2)
	{
		this->makeErrorResponse(500, NULL);
		return 404;
	}
	this->status = atoi(status_line[1].c_str());

	// Header
	std::vector<std::string> header_line;
	std::size_t header_sep = cgi_raw.find("\r\n\r\n");
	// ft_split(cgi_raw.substr(status_sep + 2, header_sep - status_sep - 2), " ", header_line);
	{
		std::istringstream	iss(cgi_raw.substr(status_sep + 2, header_sep - status_sep - 2));
		std::string			elem;
		while (iss >> elem)
			header_line.push_back(elem);
	}
	// for(std::vector<std::string>::iterator it = header_line.begin(); it != header_line.end(); it++)
	// {
	// 	std::cout << "{" << *it << "} ";
	// }
	// std::cout << std::endl;
	
	std::vector<std::string>::iterator it;
	for (it = header_line.begin(); it != header_line.end(); it++)
	{
		if((*it).compare("content-type:") == 0 || (*it).compare("Content-Type:") == 0)
		{
			if (++it != header_line.end() && (*it).at((*it).length() - 1) == ';')
				(*it).resize((*it).length()-1);
			break;
		}
	}
	if(it != header_line.end())
		this->headers.insert(std::pair<std::string, std::string>("Content-Type", *it));

	// Body
	if (cgi_raw.length() > header_sep + 4)
		this->body = cgi_raw.substr(header_sep + 4);
	else
		this->body = "";

	this->makeResponse();
	request.getConnection()->setStatus(RESPONSE_COMPLETE);
	return 0;
}

int Response::makeErrorFileResponse(int curr_event_fd)
{
	char buf[BUFFER_SIZE + 1];
	int read_size;

	read_size = read(curr_event_fd, buf, BUFFER_SIZE - 1);
	if (read_size == -1)
	{
		std::cerr << "temporary resource read error!" << std::endl;
		return 404;
	}
	buf[read_size] = 0;

	this->connection->getResponse().getBody().append(buf);
	if (read_size < BUFFER_SIZE)
	{
		std::stringstream ss;
		ss << connection->getResponse().getBody().length();
		connection->getResponse().getHeaders().insert(std::pair<std::string, std::string>("Content-Length", ss.str()));
		connection->getResponse().makeResponse();

		Webserver::getWebserverInst()->unsetFdMap(curr_event_fd);
		close(curr_event_fd);

		connection->setStatus(RESPONSE_COMPLETE);
	}
	return 0;
}

// std::map<std::string, std::string> &Response::getMimeType()
// {
// 	return (this->mime_type);
// }

std::map<std::string, std::string> &Response::getStatusCode()
{
	return (this->status_code);
}

void Response::initStatusCode(void)
{
	this->status_code["100"] = "Continue";
	this->status_code["101"] = "Switching Protocols";
	this->status_code["102"] = "Processing";
	this->status_code["200"] = "OK";
	this->status_code["201"] = "Created";
	this->status_code["202"] = "Accepted";
	this->status_code["203"] = "Non-authoritative Information";
	this->status_code["204"] = "No Content";
	this->status_code["205"] = "Reset Content";
	this->status_code["206"] = "Partial Content";
	this->status_code["207"] = "Multi-Status";
	this->status_code["208"] = "Already Reported";
	this->status_code["226"] = "IM Used";
	this->status_code["300"] = "Multiple Choices";
	this->status_code["301"] = "Moved Permanently";
	this->status_code["302"] = "Found";
	this->status_code["303"] = "See Other";
	this->status_code["304"] = "Not Modified";
	this->status_code["305"] = "Use Proxy";
	this->status_code["307"] = "Temporary Redirect";
	this->status_code["308"] = "Permanent Redirect";
	this->status_code["400"] = "Bad Request";
	this->status_code["401"] = "Unauthorized";
	this->status_code["402"] = "Payment Required";
	this->status_code["403"] = "Forbidden";
	this->status_code["404"] = "Not found";
	this->status_code["405"] = "Method Not Allowed";
	this->status_code["406"] = "Not Acceptable";
	this->status_code["407"] = "Proxy Authentication Required";
	this->status_code["408"] = "Required Timeout";
	this->status_code["409"] = "Conflict";
	this->status_code["410"] = "Gone";
	this->status_code["411"] = "Length Required";
	this->status_code["412"] = "Precondition Failed";
	this->status_code["413"] = "Request Entity Too Large";
	this->status_code["414"] = "Request URI Too Long";
	this->status_code["415"] = "Unsupported Media Type";
	this->status_code["416"] = "Requested Range Not Satisfiable";
	this->status_code["417"] = "Expectation Failed";
	this->status_code["418"] = "IM_A_TEAPOT";
	this->status_code["500"] = "Internal Server Error";
	this->status_code["501"] = "Not Implemented";
	this->status_code["502"] = "Bad Gateway";
	this->status_code["503"] = "Service Unavailable";
	this->status_code["504"] = "Gateway Timeout";
	this->status_code["505"] = "HTTP Version Not Supported";
	this->status_code["506"] = "Variant Also Negotiates";
	this->status_code["507"] = "Insufficient Storage";
	this->status_code["508"] = "Loop Detected";
	this->status_code["510"] = "Not Extened";
	this->status_code["511"] = "Network Authentication Required";
	this->status_code["599"] = "Network Connect Timeout Error";
}
