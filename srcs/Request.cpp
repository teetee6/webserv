#include "Request.hpp"
#include "Connection.hpp"

Request::Request(void)
{

	initRequest();
}

Request::Request(const Request &src)
{

	this->raw_request.clear();
	initRequest();

	this->raw_request = src.raw_request;
	this->method = src.method;
	this->uri = src.uri;
	this->http_version = src.http_version;
	this->headers.insert(src.headers.begin(), src.headers.end());

	this->raw_header = src.raw_header;
	this->raw_body = src.raw_body;
	this->temp_body = src.temp_body;
	this->parse_status = src.parse_status;
	this->body_type = src.body_type;
	this->path = src.path;
	this->connection = src.connection;
}

Request &Request::operator=(const Request &src)
{
	this->raw_request.clear();
	initRequest();

	this->raw_request = src.raw_request;
	this->method = src.method;
	this->uri = src.uri;
	this->http_version = src.http_version;
	this->headers.insert(src.headers.begin(), src.headers.end());

	this->raw_header = src.raw_header;
	this->raw_body = src.raw_body;
	this->temp_body = src.temp_body;
	this->parse_status = src.parse_status;
	this->body_type = src.body_type;
	this->path = src.path;
	this->connection = src.connection;

	return (*this);
}

///////////////////////////
/////////getter////////////
///////////////////////////

std::string &Request::getRawRequest(void)
{
	return (this->raw_request);
}

const std::string &Request::getMethod(void) const
{

	return (this->method);
}

const std::string &Request::getUri(void) const
{
	return (this->uri);
}

const std::string &Request::getHttpVersion(void) const
{
	return (this->http_version);
}

std::multimap<std::string, std::string> &Request::getHeaders(void)
{
	return (this->headers);
}

const std::string &Request::getRawHeader(void) const
{
	return (this->raw_header);
}

const std::string &Request::getRawBody(void) const
{
	return (this->raw_body);
}

const std::string &Request::getTempBody(void) const
{
	return (this->temp_body);
}

Connection *Request::getConnection(void)
{
	return (this->connection);
}

std::string &Request::getPath(void)
{
	return (this->path);
}

///////////////////////////
/////////getter_end////////
///////////////////////////

void Request::setConnection(Connection *connection)
{
	this->connection = connection;
}

void Request::setPath(const std::string &path)
{
	this->path = path;
}

void Request::setRawRequest(char *buf)
{
	this->raw_request += buf;
}

void Request::initRequest(void)
{ // request 멤버변수 초기화

	this->method.clear();
	this->uri.clear();
	this->http_version.clear();
	this->headers.clear();
	this->raw_header.clear();
	this->raw_body.clear();
	this->temp_body.clear();
	this->parse_status = PARSING_HEADER;
	this->body_type = NOBODY;
	this->path.clear();
}

// for(int i =0; i<50; i++) std::cout << "d";
// 		std::cout << std::endl;

bool Request::parseRequest(void)
{
	std::size_t index = this->raw_request.find("\r\n\r\n"); // the location of the boundary between HEADER and BODY

	if (index != std::string::npos && this->parse_status == PARSING_HEADER)
	{
		this->parseHeaders();
		this->parse_status = PARSING_BODY;
		int _body_type = this->setBodyType(); // CHUNKED or NOBODY or CONTENT_LENGTH

// for(int i =0; i<50; i++) std::cout << "d";
// 		std::cout << std::endl;
		// std::cout << this->body_type << std::endl;

		if (_body_type == NOBODY)
		{
			this->temp_body.clear();
			return (true);
		}
	}
	if (this->parse_status == PARSING_BODY)
	{
		this->temp_body += raw_request;
		raw_request.clear();
		return (parseBody()); //요청 메시지 중 body부분 데이터를 this->raw_body에 할당
	}

	return (false);
}

///////////////// private func ///////////////////////
/*
void Request::parseFirstLine(void)
{ //요청메시지 첫줄 중 공백기준 쪼갠 단어를 멤버변수에 할당 + 파싱할 요청메시지에서 첫줄을 제외한 나머지 재할당

	std::size_t index = this->raw_request.find("\r\n");
	std::string start_line = this->raw_request.substr(0, index); //요청메시지 중 첫줄

	//아래 3개 함수는 요청메시지 첫줄에서 공백기준 각 단어 구분해서 멤버변수에 할당.
	this->setMethod(start_line);
	this->setUri(start_line);
	this->setHttpVersion(start_line);

	if (this->raw_request.length() > (index + 2)) //첫번째 줄 지우고 두번째 줄부터 끝줄까지 메시지로 재할당.
		this->raw_request = this->raw_request.substr(index + 2);
	else
		this->raw_request = "";
}
*/
void Request::setMethod(std::string &start_line)
{

	this->method = start_line.substr(0, start_line.find(' '));
}

void Request::setUri(std::string &start_line)
{

	std::size_t start_pos = start_line.find(' ');

	this->uri = start_line.substr(start_pos + 1, start_line.rfind(' ') - start_pos - 1);
}

void Request::setHttpVersion(std::string &start_line)
{

	this->http_version = start_line.substr(start_line.rfind(' ') + 1);
}

void Request::parseHeaders(void)
{
	std::size_t start_line_end = this->raw_request.find("\r\n");
	std::string start_line = this->raw_request.substr(0, start_line_end);

	// first line
	this->setMethod(start_line);
	this->setUri(start_line);
	this->setHttpVersion(start_line);

	// header line
	if (this->raw_request.length() > (start_line_end + 2))
		this->raw_request = this->raw_request.substr(start_line_end + 2);
	else
		this->raw_request = "";

	this->raw_header = this->raw_request.substr(0, this->raw_request.find("\r\n\r\n") + 2); //요청메시지 맨끝에 \r\n포함

	while (this->raw_header.length())
	{
		std::size_t line_end = this->raw_header.find("\r\n");
		if (line_end == std::string::npos)
			break;
		std::string line = this->raw_header.substr(0, line_end);
		std::size_t idx = line.find(":");
		std::string key = line.substr(0, idx);
		std::string value = "";
		while (idx + 1 < line_end && line[idx + 1] == ' ')
			idx++;
		if (line_end > idx + 1)
			value = line.substr(idx + 1);
		this->headers.insert(std::pair<std::string, std::string>(key, value));
		this->raw_header.erase(0, line_end + 2);

		if (line.find("multipart/form-data;") != std::string::npos && line.find("boundary=") != std::string::npos)
			this->headers.insert(std::pair<std::string, std::string>("boundary", line.substr(line.find_last_of("boundary=") + 9)));
	}

	size_t header_end = this->raw_request.find("\r\n\r\n");

	if (this->raw_request.length() > header_end + 4)
	{	
		this->raw_request = this->raw_request.substr(header_end + 4);
		if (this->raw_request.find(this->getHeaders().count("boundary")))
		{
			this->raw_request = this->raw_request.substr(this->raw_request.find("\r\n") + 2);
			std::string tmp = this->raw_request.find("Content-Type: " + 14);
		}
	}
	else
		this->raw_request.clear();

	for(std::multimap<std::string, std::string>::iterator it= this->headers.begin(); it != this->headers.end(); it++)
	{
		std::cout << "[[[[" << it->first << ", " << it->second << "]]]]\n";
	}
}

// python(or php)에서 upload file 받아 처리하는거 multer같은거 찾아보자
bool Request::setBodyType(void)
{
	std::map<std::string, std::string>::iterator iter = this->headers.find("Transfer-Encoding");
	if (iter != this->headers.end() && iter->second == "chunked")
		return (this->body_type = CHUNKED);

	iter = this->headers.find("content-length");
	if (iter != this->headers.end() && iter->second != "")
	{
		return (this->body_type = CONTENT_LENGTH);
	}
	return (this->body_type); // nothing
}

// only fill into this->rawbody
bool Request::parseBody(void)
{
	// std::cout << "바디파싱0\n";

	std::multimap<std::string, std::string>::iterator iter = this->headers.find("content-length"); // map 으로 바꾸자
	std::size_t content_length = 0;
	
	if (iter != this->headers.end())
		content_length = atoi(iter->second.c_str());

	if (this->body_type == CONTENT_LENGTH && this->temp_body.length() >= content_length)
	{
		// std::cout << "ㅂㅡ디파싱1\n";
		
		this->raw_body += this->temp_body.substr(0, content_length);
		temp_body.clear();
		// std::cout << "확인용333\n";
		// std::cout << this->temp_body << std::endl;
		// std::cout << "확인용444\n";
		this->parse_status = PARSING_HEADER;
		return (true);
	}
	
	if (this->body_type == CHUNKED)
	{
		// std::cout << "ㅂㅡ디파싱2\n";
		std::size_t index = this->temp_body.find("\r\n");
		std::size_t chunk_size;

		while (index != std::string::npos)
		{
			chunk_size = static_cast<size_t>(strtol(this->temp_body.c_str(), NULL, 16));

			if (chunk_size == 0){
				if (temp_body.length() == 5){
					this->temp_body.clear();
					this->parse_status = PARSING_HEADER;
					return (true);
				}
				else
					return (false);
			}
			
			std::string body_after_chunk_size = this->temp_body.substr(index + 2);
			if (body_after_chunk_size.length() >= chunk_size + 2)
			{	
				this->raw_body += body_after_chunk_size.substr(0, chunk_size);
				this->temp_body.clear();
				if (body_after_chunk_size.length() > chunk_size + 2)
					this->temp_body = body_after_chunk_size.substr(chunk_size + 2);
			}
			else
				break;
			index = this->temp_body.find("\r\n");
		}
	}
	// std::cout << "ㅂㅡ디파싱3\n";
	
	return (false);
}
