#include "Request.hpp"
#include "Connection.hpp"
#include "Webserver.hpp"

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

int Request::getBodyType(void)
{
	return (this->body_type);
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

void Request::setRawRequest(std::string str)
{
	this->raw_request += str;
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

bool Request::parseRequest(void)
{
	std::size_t index = this->raw_request.find("\r\n\r\n"); // the location of the boundary between HEADER and BODY

	if (index != std::string::npos && this->parse_status == PARSING_HEADER)
	{
		this->parseHeaders();
		this->parse_status = PARSING_BODY;
		int _body_type = this->setBodyType(); // CHUNKED or NOBODY or MULTIPART or CONTENT_LENGTH
		if (_body_type == NOBODY)
		{
			this->temp_body.clear();
			return (true);
		}
	}
	if (this->parse_status == PARSING_BODY)
	{
		this->temp_body += this->raw_request;
		this->raw_request.clear();
		bool is_parse_end = parseBody(); //요청 메시지 중 body부분 데이터를 this->raw_body에 할당
		return (is_parse_end);
	}

	return (false);
}

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
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		this->headers.insert(std::pair<std::string, std::string>(key, value));
		this->raw_header.erase(0, line_end + 2);

		if (line.find("multipart/form-data;") != std::string::npos && line.find("boundary=") != std::string::npos)
		{
			size_t boundary_idx = line.find("boundary=");
			this->headers.insert(std::pair<std::string, std::string>("boundary", line.substr(boundary_idx + 9)));
			std::string multipart_type_str = this->headers.lower_bound("content-type")->second;
			multipart_type_str = multipart_type_str.substr(0, multipart_type_str.find(";"));
			this->headers.lower_bound("content-type")->second = multipart_type_str;
		}
	}

	size_t header_end = this->raw_request.find("\r\n\r\n");

	if (this->raw_request.length() > header_end + 4)
		this->raw_request = this->raw_request.substr(header_end + 4);
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
	std::map<std::string, std::string>::iterator iter;
	iter = this->headers.find("content-type");
	if (iter != this->headers.end() && iter->second == "multipart/form-data")
		return (this->body_type = MULTIPART);

	iter = this->headers.find("transfer-encoding");
	if (iter != this->headers.end() && iter->second == "chunked")
		return (this->body_type = CHUNKED);


	iter = this->headers.find("content-length");
	if (iter != this->headers.end())
		return (this->body_type = CONTENT_LENGTH);
	return (this->body_type); // nothing
}

// {
// 	"data" :
// 	[
// 		{
// 			"name": "imgFile",
// 			"filename": "hyoslee.jpeg",
// 			"content_type": "image/jpeg",
// 			"file_location": "./upload/hyoslee.jpeg"
// 		},
// 		{
// 			"name": "justFile",
// 			"filename": "a.out",
// 			"content_type": "application/octet-stream",
// 			"file_location": "./upload/a.out"
// 		},
// 		{
// 			"name": "thisIsKey",
// 			"value": "hahaha"
// 		}
// 	]
// }

void Request::parseMultipart(void)
{
	std::string boundary = this->headers.lower_bound("boundary")->second;
	std::string boundary_end = boundary + "--";
	std::vector<std::pair<std::string, std::string> > upload_file_list;
		
	std::string raw = this->getRawBody();
	std::string json = "{\"data\" :[";
	size_t interval_idx = raw.find(boundary);
	raw.erase(0, interval_idx + boundary.length() + 2);
	while(1)
	{
		interval_idx = raw.find(boundary);
		if (interval_idx == std::string::npos)
		{
			if (json[json.length() - 1] == ',')
				json.resize(json.length() - 1);
			json += std::string("]}");
			break;
		}

		json += std::string("{");
		size_t header_end = raw.find("\r\n\r\n");
		size_t data_start = header_end + 4;
		size_t data_end = interval_idx;
		while (raw[data_end] == '-')
			--data_end;
		--data_end;
		std::string raw_head = raw.substr(0, header_end);
		std::string raw_data = raw.substr(data_start, data_end - data_start);

		size_t name_s = raw_head.find("name=\"");
		std::string name;
		if (name_s != std::string::npos)
		{
			name_s += 6;
			size_t name_e = raw_head.find("\"", name_s + 1);
			name = raw_head.substr(name_s, name_e - name_s);
			json += std::string("\"name\": ") + std::string("\"") + name + std::string("\"");
			json += std::string(",");
		}

		size_t filename_s = raw_head.find("filename=\"");
		std::string filename;
		if (filename_s != std::string::npos)
		{
			filename_s += 10;
			size_t filename_e = raw_head.find("\"", filename_s + 1);
			filename = raw_head.substr(filename_s, filename_e - filename_s);
			json += std::string("\"filename\": ") + std::string("\"") + filename + std::string("\"");
			json += std::string(",");
		}

		size_t content_type_s = raw_head.find("Content-Type: ");
		if (content_type_s == std::string::npos)
			content_type_s = raw_head.find("content-type: ");
		std::string content_type;
		if (content_type_s != std::string::npos)
		{
			content_type_s += 14;
			size_t content_type_e = raw_head.find_first_of(";\r\n", content_type_s);
			content_type = raw_head.substr(content_type_s, content_type_e - content_type_s);
			json += std::string("\"content_type\": ");
			json += std::string("\"") + content_type + std::string("\"");
			json += std::string(",");
		}

		if(filename.length())
		{
			static char buf[1024];
			static bool identify_upload_folder = false;
			if (!identify_upload_folder)
			{
				realpath("./upload/", buf);
				identify_upload_folder = true;
			}
			std::string tmp_filename = std::string(buf) + std::string("/") + std::string("tmp__") + filename;
			upload_file_list.push_back(std::pair<std::string, std::string>(tmp_filename ,raw_data));

			json += std::string("\"file_location\": ");
			json += std::string("\"") + tmp_filename + std::string("\"");
			json += std::string(",");
		}
		else
		{
			json += std::string("\"value\": ") + std::string("\"") + raw_data + std::string("\"");
			json += std::string(",");
		}
		if (json[json.length() - 1] == ',')
			json.resize(json.length() - 1);
		json += std::string("},");
		raw.erase(0, interval_idx + boundary.length() + 2);
	}
	std::cout << json << std::endl;
	this->raw_body = json;
	
	if (upload_file_list.size() == 0)
		this->connection->getResponse().makeResponseMultipart();
	else
	{
		KqueueMonitoredFdInfo *multipart_fdtype = new KqueueMonitoredFdInfo(UPLOAD_FILE_FDTYPE, this->getConnection(), upload_file_list);
		std::map<pid_t, std::pair<std::string, size_t> > upload_fds = multipart_fdtype->getUploadFds();
		for (std::map<pid_t, std::pair<std::string, size_t> >::iterator it = upload_fds.begin(); it != upload_fds.end(); it++)
		{
			pid_t upload_fd = it->first;
			Webserver::getWebserverInst()->setFdMap(upload_fd, multipart_fdtype);
			Webserver::getWebserverInst()->getKq().createChangeListEvent(upload_fd, "W");
		}
	}
}


// only fill into this->rawbody
bool Request::parseBody(void)
{
	std::multimap<std::string, std::string>::iterator iter;
	std::size_t content_length = 0;
	
	iter = this->headers.find("content-length");
	if (iter != this->headers.end())
		content_length = atoi(iter->second.c_str());

	std::cout << this->temp_body.length() << ", total content_length: " << content_length << std::endl;

	if (this->body_type == MULTIPART || this->body_type == CONTENT_LENGTH)
	{
		if (this->temp_body.length() >= content_length)
		{
			this->raw_body += this->temp_body.substr(0, content_length);
			this->temp_body.clear();
			this->parse_status = PARSING_HEADER;
			
			std::cout << "\x1b[33m" << "파싱 끝났어 진행해\n" << "\x1b[0m";
			std::cout << "\x1b[32m""[complete data]----------------------------------------\n";
			std::cout << this->getRawBody() << std::endl;
			std::cout << this->getRawBody().length() << std::endl;
			std::cout << "----------------------------------------[complete data-here]\n""\x1b[0m";
			return (true);
		}
		return (false);
	}

	if (this->body_type == CHUNKED)
	{
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
	
	return (false);
}
