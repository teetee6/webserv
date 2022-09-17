#include "Cgi.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Location.hpp"
#include "Connection.hpp"
#include "FdType.hpp"
#include "Webserver.hpp"

Cgi::Cgi(void)
{
	this->pid = 0;
}

Cgi::~Cgi(void) {}

void Cgi::cgiPipeFdSet(Request &request, Location &location, std::string &file_name, const std::string &ext_path)
{
	bool is_post = false;
	for(std::list<std::string>::iterator it = location.getAllowMethods().begin(); it != location.getAllowMethods().end(); it++)
	{
		if((*it).compare("POST") == 0 || (*it).compare("PUT") == 0)
			is_post = true;
	}

	if (pipe(this->response_fd) == -1 || (is_post && pipe(this->request_fd) == -1))
	{
		std::cerr << "pipe() failed" << strerror(errno) << std::endl;
		request.getConnection()->getResponse().makeErrorResponse(500, &location);
		return;
	}

	this->pid = fork();
	if (this->pid < 0)
	{
		std::cerr << "CGI " << file_name << ": fork() error" << std::endl;
		request.getConnection()->getResponse().makeErrorResponse(500, &location);
		return;
	}
	else if (this->pid == 0)
	{
		std::string file_path = request.getUri();

		//url뒷부분 ex. asdf.bla
		file_path = file_path.substr(location.getLocationName().length()); // location경로
		file_path = location.getRoot() + file_path; // root+location경로

		char *buf = realpath(file_path.c_str(), NULL); //절대경로로 변환
		if (buf != NULL)
			file_path = std::string(buf);
		free(buf);

		char **env = this->setCgiEnvironment(request, location, file_path);

		if (is_post)
		{
			close(this->request_fd[1]);
			dup2(this->request_fd[0], 0);
		}
		close(this->response_fd[0]);
		dup2(this->response_fd[1], 1);
		if (file_name.substr(file_name.rfind('.')) == ".bla")
		{
			char **lst = (char **)malloc(sizeof(char *) * 2);
			lst[0] = strdup("./cgi-bin/cgi_tester");
			lst[1] = NULL;
			if (execve("./cgi-bin/cgi_tester", lst, env) == -1)
			{
				std::cerr << "CGI EXECUTE ERROR: " << strerror(errno) << std::endl;
				exit(1);
			}
		}
		else
		{
			char **lst = (char **)malloc(sizeof(char *) * 3);
			lst[0] = strdup(ext_path.c_str());
			lst[1] = strdup(file_path.c_str());
			lst[2] = NULL;
			if (execve(ext_path.c_str(), lst, env) == -1)
			{
				std::cerr << "CGI EXECUTE ERROR: " << strerror(errno) << std::endl;
				exit(1);
			}
		}
		exit(0);
	}
	else
	{
		if (is_post)
		{
			close(this->request_fd[0]);
			fcntl(this->request_fd[1], F_SETFL, O_NONBLOCK);
		}
		close(this->response_fd[1]);
		fcntl(this->response_fd[0], F_SETFL, O_NONBLOCK);

		if (is_post)
		{
				std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!DATA!!!!!!\n";
				std::cout << request.getRawBody() << std::endl;
				std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";

			FdType *pipe_fd = new FdType(CGI_WRITE_FDTYPE, request.getConnection(), this->pid, request.getRawBody());
			Webserver::getWebserverInst()->setFdMap(this->request_fd[1], pipe_fd);
			Webserver::getWebserverInst()->getKq().createChangeListEvent(this->request_fd[1], "W");
		}
		FdType *resource_fd = new FdType(CGI_READ_FDTYPE, request.getConnection(), this->pid);
		Webserver::getWebserverInst()->setFdMap(this->response_fd[0], resource_fd);
		Webserver::getWebserverInst()->getKq().createChangeListEvent(this->response_fd[0], "R");

		return;
	}
}

int *Cgi::getRequestFD(void) const
{
	return (const_cast<int *>(this->request_fd));
}

int *Cgi::getResponseFD(void) const
{
	return (const_cast<int *>(this->response_fd));
}

// please trim
char **Cgi::setCgiEnvironment(Request &request, Location &location, std::string &file_path)
{ // header처럼 cgi정보 할당한 내용을 동적할당 시켜서 리턴


	std::map<std::string, std::string> cgi_env;

	// authorization 을 cgi_env에 Insert
	std::multimap<std::string, std::string>::iterator iter;// = request.getHeaders().find("Authorization");
	// if (iter != request.getHeaders().end() && iter->second != "")
	// {
	// 	std::size_t found = iter->second.find(' ');
	// 	cgi_env.insert(std::pair<std::string, std::string>("AUTH_TYPE", iter->second.substr(0, found)));
	// }

	// content_length 을 cgi_env에 Insert
	iter = request.getHeaders().find("content-length");
	if (iter != request.getHeaders().end() && iter->second != "")
		cgi_env.insert(std::pair<std::string, std::string>("CONTENT_LENGTH", iter->second));
	else if (((iter = request.getHeaders().find("Transfer-Encoding")) != request.getHeaders().end()) && iter->second == "chunked")
	{
		std::stringstream ss;
		std::string str;
		ss << request.getRawBody().length();
		ss >> str;
		std::cout << "asdjklasdjaskldjaskldjaklsjdklasdjklasd" << std::endl;
		cgi_env.insert(std::pair<std::string, std::string>("CONTENT_LENGTH", str));
	}
	else
		cgi_env.insert(std::pair<std::string, std::string>("CONTENT_LENGTH", "0"));

	// content_type 을 cgi_env에 Insert
	iter = request.getHeaders().find("Content-Type");
	if (iter != request.getHeaders().end() && iter->second != "")
		cgi_env.insert(std::pair<std::string, std::string>("CONTENT_TYPE", iter->second));
	
	cgi_env.insert(std::pair<std::string, std::string>("GATEWAY_INTERFACE", "Cgi/1.1"));

	// path_info 을 cgi_env에 Insert
	// path_translated  을 cgi_env에 Insert
	std::size_t back_pos = request.getUri().find('?');
	std::size_t front_pos = request.getUri().rfind('.', back_pos);
	std::string path_info = request.getUri().substr(front_pos, back_pos - front_pos);

	if ((front_pos = path_info.find('/')) != std::string::npos)
		path_info = path_info.substr(front_pos);
	else
		path_info = request.getUri();

	cgi_env.insert(std::pair<std::string, std::string>("PATH_INFO", path_info)); // 잠시

	cgi_env.insert(std::pair<std::string, std::string>("PATH_TRANSLATED", location.getRoot() + path_info.substr(1)));

	// query_string 을 cgi_env에 Insert
	if (request.getUri().find('?') != std::string::npos)
		cgi_env.insert(std::pair<std::string, std::string>("QUERY_STRING", request.getUri().substr(request.getUri().find('?'))));

	// request_method와 request_uri 을 cgi_env에 Insert
	cgi_env.insert(std::pair<std::string, std::string>("REQUEST_METHOD", request.getMethod()));
	cgi_env.insert(std::pair<std::string, std::string>("REQUEST_URI", request.getUri()));

	// script_name 을 cgi_env에 Insert
	if (request.getUri() == path_info)
		cgi_env.insert(std::pair<std::string, std::string>("SCRIPT_NAME", location.getRoot() + path_info.substr(1)));
	else if (request.getUri().find(path_info) == std::string::npos)
	{
		cgi_env.insert(std::pair<std::string, std::string>("SCRIPT_NAME", location.getRoot() + request.getUri().substr(1))); // 임시
	}
	else
	{
		std::size_t pos = request.getUri().rfind(path_info);
		cgi_env.insert(std::pair<std::string, std::string>("SCRIPT_NAME", location.getRoot() + request.getUri().substr(1, pos - 1)));
	}

	// Script_filename  을 cgi_env에 Insert
	size_t pos = file_path.find(".");
	size_t pos2 = file_path.find("/", pos);
	if (pos2 == std::string::npos)
		pos2 = file_path.find("?", pos);
	cgi_env.insert(std::pair<std::string, std::string>("SCRIPT_FILENAME", file_path.substr(0, pos2)));

	// server정보를 cgi_env에 Insert
	cgi_env.insert(std::pair<std::string, std::string>("SERVER_NAME", "127.0.0.1"));
	{
		std::stringstream ss;
		std::string str;
		ss << request.getConnection()->getServer()->getPort();
		ss >> str;
		cgi_env.insert(std::pair<std::string, std::string>("SERVER_PORT", str));
	}
	cgi_env.insert(std::pair<std::string, std::string>("SERVER_PROTOCOL", "HTTP/1.1"));
	cgi_env.insert(std::pair<std::string, std::string>("SERVER_SOFTWARE", "AeronHyosi/1.0"));

	// http Header 을 cgi_env에 Insert
	for (std::map<std::string, std::string>::const_iterator iter = request.getHeaders().begin(); iter != request.getHeaders().end(); iter++)
	{
		cgi_env.insert(std::pair<std::string, std::string>("HTTP_" + iter->first, iter->second));
	}
	return (makeCgiEnvironment(cgi_env));
}

char **Cgi::makeCgiEnvironment(std::map<std::string, std::string> &cgi_env)
{ // header처럼 cgi정보 할당한 내용을 키 = 밸류 형태로 바꾸고 동적할당 시키고 리턴

	char **env;
	int i = 0;

	env = (char **)malloc(sizeof(char *) * (cgi_env.size() + 1));
	for (std::map<std::string, std::string>::iterator iter = cgi_env.begin(); iter != cgi_env.end(); iter++)
	{
		std::string tmp = std::string(iter->first + "=" + iter->second);
		env[i] = strdup(tmp.c_str());
		i++;
	}
	env[i] = NULL;
	return (env);
}

