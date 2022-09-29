#include "Webserver.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Connection.hpp"

Webserver *Webserver::webserver_inst;

Webserver::Webserver() {}

Webserver::~Webserver() {}

Webserver *Webserver::getWebserverInst()
{
	if (webserver_inst == NULL)
		webserver_inst = new Webserver();
	return (webserver_inst);
}

std::vector<Server> &Webserver::getRealServer()
{
	return (this->real_server);
}

bool Webserver::initKqueue()
{
	if ((this->kq.setKqueue(kqueue())) == -1)
		throw "kqueue() error";
	return true;
}

void Webserver::setFdMap(int fd, KqueueMonitoredFdInfo *FdInstance)
{
	this->fd_map.insert(std::pair<int, KqueueMonitoredFdInfo *>(fd, FdInstance));
}

// delete and close the fd from Webserver
void Webserver::unsetFdMap(int fd)
{
	std::map<int, KqueueMonitoredFdInfo *> _fd_map = this->getFdMap();
	std::map<int, KqueueMonitoredFdInfo *>::iterator iter = _fd_map.find(fd);
	if (iter != this->getFdMap().end())
	{
		delete iter->second;
		iter->second = NULL;
	}
	this->getFdMap().erase(fd);
}

std::map<int, KqueueMonitoredFdInfo *> &Webserver::getFdMap()
{
	return this->fd_map;
}

bool Webserver::initServers()
{
	for (std::vector<Server>::iterator iter = this->getRealServer().begin(); iter != this->getRealServer().end(); iter++)
	{
		struct sockaddr_in server_addr;
		int server_socket = socket(PF_INET, SOCK_STREAM, 0);
		if (server_socket == -1)
			throw "socket() error";

		int option_value = 1; // set option(SO_REUSEADDR) ON(1)
		setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));

		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = inet_addr(iter->getIP().c_str());
		server_addr.sin_port = htons(iter->getPort());

		if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
			throw "bind() error";
		if (listen(server_socket, 2048) == -1)
			throw "listen() error";
		{
			std::cout << "Server ";
			std::cout << iter->getServerName();
			std::cout << "(" << iter->getIP() << ":" << iter->getPort() << ") ";
			std::cout << "started" << std::endl;
		}
		this->servers_fd[server_socket] = &(*iter);

		KqueueMonitoredFdInfo *server_fd_instance = new KqueueMonitoredFdInfo(SERVER_FDTYPE);
		setFdMap(server_socket, server_fd_instance);
		this->servers_fd[server_socket]->setSocketFd(server_socket);
		this->kq.createChangeListEvent(server_socket, "R");
	}
	return (true);
}

Kqueue &Webserver::getKq()
{
	return this->kq;
}

// uri = /dobby/hello?ha=hi2/index.html
Location &Webserver::findLocation(Server &server, const std::string &uri)
{
	size_t pos;
	std::string uri_loc = "";

	pos = uri.find('.');
	if (pos != std::string::npos)
	{
		while (uri[pos] != '/')
			pos--;
		uri_loc = uri.substr(0, pos + 1); // /dobby/hello?ha=hi2/
	}
	else
	{
		pos = uri.find('?');
		if (pos != std::string::npos)
			uri_loc = uri.substr(0, pos); // /dobby/hello
		else
			uri_loc = uri;
	}

	if (uri_loc[uri_loc.length() - 1] != '/') // /dobby/hello/
		uri_loc += "/";

	std::map<std::string, Location> &loc_map = server.getLocations();
	Location *ret = &loc_map["/"]; // default return

	std::string key = "";
	// /abc/def
	for (std::string::const_iterator iter = uri_loc.begin(); iter != uri_loc.end(); iter++)
	{
		key += *iter;
		if (*iter == '/')
		{
			if (loc_map.find(key) == loc_map.end())
				return (*ret);
			else
				ret = &loc_map[key];
		}
	}
	return (*ret);
}

int Webserver::isValidRequestwithConfig(Connection &connection)
{
	std::cout << "isValidRequestwithConfig" << std::endl;
	Location &location = this->findLocation(*connection.getServer(), connection.getRequest().getUri());

	if (std::find(location.getAllowMethods().begin(), location.getAllowMethods().end(), connection.getRequest().getMethod()) == location.getAllowMethods().end())
	{
		std::cout << "isValidRequestwithConfig - Not allowed Method" << std::endl;
		connection.getResponse().makeErrorResponse(405, &location); // HAKLA 같이 이상한 요청방식 보냈을떄
		return (405);
	}

	if (connection.getRequest().getRawBody().length() > static_cast<size_t>(location.getBodyLimitSize()))
	{
		std::cout << "isValidRequestwithConfig - bodyLimit!" << std::endl;
		connection.getResponse().makeErrorResponse(413, &location);
		return (413);
	}

	return 0;
}

int Webserver::isMultipart(Connection &connection, Location &location)
{
	std::cout << "isMultipart" << std::endl;
	if (connection.getRequest().getBodyType() == MULTIPART)
	{
		for (std::list<std::string>::const_iterator iter = location.getAllowMethods().begin(); iter != location.getAllowMethods().end(); iter++)
		{
			if (*iter == "POST")
			{
				connection.getRequest().parseMultipart();
				return (0);
			}
		}
	}

	return (1);
}

int Webserver::isRedirect(Connection &connection, Location &location)
{
	std::cout << "isRedirect" << std::endl;
	int is_redirect = location.getRedirectReturn();
	if (is_redirect == -1)
		return 1;
	connection.getResponse().makeRedirectResponse(location);
	return 0;
}

bool Webserver::isCgi(Location &location, Request &request)
{
	std::cout << "isCgi" << std::endl;
	std::map<std::string, std::string> &cgi_paths = location.getCgiPaths();

	size_t dot_index = request.getUri().find('.'); // 14 (/cgi_bin/youpi.out)
	if (dot_index == std::string::npos)
		return (false);
	size_t end_index = dot_index;

	// . 부터 / 나 ? 이전까지, 없으면 끝까지 인덱스++
	while (end_index != request.getUri().length() && request.getUri()[end_index] != '/' && request.getUri()[end_index] != '?')
		end_index++;

	std::string extension = request.getUri().substr(dot_index, end_index - dot_index); // .out

	std::map<std::string, std::string>::const_iterator extension_in_location;

	//요청들어온 Cgi uri와 location cgi uri 비교
	if ((extension_in_location = cgi_paths.find(extension)) == cgi_paths.end())
		return (false);
	while (request.getUri()[dot_index] != '/')
		dot_index--;
	std::string cgi_filename = request.getUri().substr(dot_index + 1, end_index - dot_index - 1); // youpi.out

	Cgi cgi;
	cgi.cgiPipeFdSet(request, location, cgi_filename, extension_in_location->second);

	return (true);
}

int Webserver::isAutoIndex(Connection &connection, Location &location)
{
	std::cout << "isAutoIndex" << std::endl;
	if (location.getAutoIndex() != true)
		return 0;

	if (connection.getRequest().getMethod() == "GET")
	{
		std::string uri = connection.getRequest().getUri().substr(0, connection.getRequest().getUri().find('?')); //요청 uri중 ? 이전까지.
		std::string path;
		// std::cout << connection.getRequest().getUri() << std::endl;

		if (uri == "/")
			path = location.getRoot();
		else
		{
			if (uri == location.getLocationName().substr(0, location.getLocationName().length() - 1))
				path = location.getRoot() + uri.substr(location.getLocationName().length() - 1);
			else
				path = location.getRoot() + uri.substr(location.getLocationName().length());
		}

		int is_dir = this->isDirectoryName(path);
		if (is_dir == -1)
		{
			// std::cout << "is_dir = -1\n";
			// connection.getResponse().makeErrorResponse(404, &location);
			// return 404;
			return 0;
		}
		else if (is_dir == false)
		{
			// std::cout << "is_dir = 0\n";
			// connection.getResponse().makeErrorResponse(400, &location);
			// return 400;
			return 0; // it is just file. so don't handle here
		}
		// std::cout << "is_dir = 1\n";

		// check if there is index file to read possible
		if (connection.getRequest().getUri() == location.getLocationName())
		{
			for (std::list<std::string>::iterator iter = location.getIndex().begin(); iter != location.getIndex().end(); iter++)
			{
				if (access((path + (*iter)).c_str(), F_OK | R_OK) == 0)
				{
					std::cout << "found index file \n";
					return 0;
				}
			}
		}
		std::cout << "it's time to check autoIndex on \n";

		if (location.getAutoIndex())
		{
			// std::cout << "make autoIndex" << std::endl;
			connection.getResponse().makeAutoIndexResponse(path, connection.getRequest().getUri(), location);
			return (AUTOINDEX_RESPONSE);
		}
		else
		{
			// // std::cout << "no autoIndex" << std::endl;
			connection.getResponse().makeErrorResponse(404, &location);
			return 404;
		}
	}
	return 0;
}

std::string Webserver::isValidIndexFile(std::string path, Location &location)
{
	std::string res;
	struct stat sb;

	for (std::list<std::string>::const_iterator iter = location.getIndex().begin(); iter != location.getIndex().end(); iter++)
	{
		res = path;
		res += *iter;

		if (stat(res.c_str(), &sb) == 0)
			return res;
	}
	return "404";
}

// 0 : path비포함(루트라서) 삭제
// 1 : path포함 삭제
int unlinkFileRmdirFolder(std::string dir, int level)
{ // 재귀로써 실질적인 폴더와 파일 삭제
	DIR *dir_ptr;
	struct dirent *file;

	if ((dir_ptr = opendir(dir.c_str())) == NULL)
		return (404);
	if (dir[dir.length() - 1] != '/')
		dir += '/';
	std::string name;
	int ret;
	while ((file = readdir(dir_ptr)) != NULL)
	{
		name = std::string(file->d_name);
		if (name == "." || name == "..")
			continue;
		if (file->d_type == DT_DIR)
		{
			ret = unlinkFileRmdirFolder(dir + name, level + 1);
			if (ret == 404)
			{
				closedir(dir_ptr);
				return (404);
			}
		}
		else
			unlink((dir + name).c_str());
	}
	if (level > 0)
		rmdir(dir.c_str());
	closedir(dir_ptr);
	return (200);
}

// delete inner files and folders, except the root folder

// 0 : path비포함(루트라서) 삭제
// 1 : path포함 삭제
int Webserver::unlinkFileAndFolder(std::string path, int level)
{
	DIR *dir_ptr = opendir(path.c_str());
	if (dir_ptr == NULL)
		return 404;
	int ret_code = unlinkFileRmdirFolder(path, level);
	closedir(dir_ptr);
	return (ret_code);
}

int Webserver::defaultToHttpMethod(Connection &connection, Location &location)
{
	std::cout << RED "defaultToHttpMethod" RESET << std::endl;
	std::string uri = connection.getRequest().getUri().substr(0, connection.getRequest().getUri().find('?')); //요청 uri중 ? 이전까지.
	std::string path;

	if (uri == "/")
		path = location.getRoot();
	else
	{
		if (uri == location.getLocationName().substr(0, location.getLocationName().length() - 1))
			path = location.getRoot() + uri.substr(location.getLocationName().length() - 1);
		else
			path = location.getRoot() + uri.substr(location.getLocationName().length());
	}

	if (connection.getRequest().getMethod() == "GET")
	{
		std::cout << "here - GET" << std::endl;
		switch (this->isDirectoryName(path))
		{
		case -1:
		{
			connection.getResponse().makeErrorResponse(404, &location);
			return 404;
			break;
		}
		case false:
			std::cout << "not directory" << std::endl;
			break;
		case true:
		{
			std::cout << "is directory" << std::endl;
			if (path[path.length() - 1] != '/')
				path += "/";
			// std::cout << path <<std::endl;
			std::string res = this->isValidIndexFile(path, location); // index or autoindex(파일열림체크) or error 체크
			if (res == "404")
			{
				connection.getResponse().makeErrorResponse(404, &location);
				return (404);
			}
			struct stat sb;
			stat(res.c_str(), &sb);
			if (S_ISDIR(sb.st_mode))
			{
				connection.getResponse().makeErrorResponse(500, &location);
				return (500);
			}
			else
				path = res;
		}
		}
		// from now one, path is file type.
		connection.getRequest().setPath(path);
		int file_fd = open(path.c_str(), O_RDONLY);
		if (file_fd == -1)
		{
			connection.getResponse().makeErrorResponse(500, &location);
			return (500);
		}

		KqueueMonitoredFdInfo *file_fd_instance = new KqueueMonitoredFdInfo(FILE_FDTYPE, &connection);
		setFdMap(file_fd, file_fd_instance);
		this->getKq().createChangeListEvent(file_fd, "R");
	}
	else if (connection.getRequest().getMethod() == "POST")
	{
		if (path == location.getRoot())
		{
			std::cout << "here" << std::endl;
			std::string res = this->isValidIndexFile(path, location); // index or autoindex(파일열림체크) or error 체크
			if (res == "404")
			{
				std::cout << "here2" << std::endl;
				connection.getResponse().makeErrorResponse(404, &location);
				return (404);
			}
			path = res;

			connection.getRequest().setPath(path); // 완전한 파일경로

			int file_fd = open(path.c_str(), O_RDONLY);
			if (file_fd == -1)
			{
				std::cout << "here3" << std::endl;
				connection.getResponse().makeErrorResponse(500, &location);
				return (500);
			}
			std::cout << "here4" << std::endl;
			KqueueMonitoredFdInfo *file_fd_instance = new KqueueMonitoredFdInfo(FILE_FDTYPE, &connection);
			setFdMap(file_fd, file_fd_instance);
			this->getKq().createChangeListEvent(file_fd, "R");
			return 0;
		}

		if(path[path.length() - 1] == '/')
		{
			connection.getResponse().makeErrorResponse(400, &location);
			return (400);			
		}

		// if (connection.getRequest().getRawBody().length() == 0)
		// {
		// 	connection.getResponse().makePostPutResponse();
		// 	return 0;
		// }

		// if (connection.getRequest().getRawBody().find("{\"data\" :[") != std::string::npos)
		if (connection.getRequest().getBodyType() == MULTIPART)
		{
			connection.getResponse().makePostPutResponse();
			return 0;
		}

		if (this->isDirectoryName(path) >= 0)
		{
			std::cout << "already Exists!!\n";
			connection.getResponse().makeErrorResponse(400, &location);
			return (400);
		}

		int put_fd = this->createFileWithSetup(path); //폴더를 만들고 그걸 연 fd를 리턴하네
		if (put_fd == -1)
		{
			connection.getResponse().makeErrorResponse(500, &location);
			return (500);
		}
		KqueueMonitoredFdInfo *file_fd_inst = new KqueueMonitoredFdInfo(FILE_FDTYPE, &connection, connection.getRequest().getRawBody());
		this->setFdMap(put_fd, file_fd_inst);
		Webserver::getWebserverInst()->getKq().createChangeListEvent(put_fd, "W");
	}
	else if (connection.getRequest().getMethod() == "PUT")
	{ // PUT 요청일때
		std::cout << path << std::endl;

		if(path[path.length() - 1] == '/')
		{
			connection.getResponse().makeErrorResponse(400, &location);
			return (400);			
		}

		if (this->isDirectoryName(path) == true)
		{
			connection.getResponse().makeErrorResponse(400, &location);
			return (400);
		}

		// from now one, path is file type.
		connection.getRequest().setPath(path);
		int put_fd = this->createFileWithSetup(path); //폴더를 만들고 그걸 연 fd를 리턴하네
		if (put_fd == -1)
		{
			connection.getResponse().makeErrorResponse(500, &location);
			return (500);
		}
		KqueueMonitoredFdInfo *file_fd_inst = new KqueueMonitoredFdInfo(FILE_FDTYPE, &connection, connection.getRequest().getRawBody());
		setFdMap(put_fd, file_fd_inst);
		Webserver::getWebserverInst()->getKq().createChangeListEvent(put_fd, "W");
	}
	else if (connection.getRequest().getMethod() == "DELETE")
	{
		std::string uri_folder = uri;
		if (uri[uri.length() - 1] != '/')
			uri_folder = uri + "/";

		// uri matches to the root path of location
		if (uri_folder == location.getLocationName())
		{
			connection.getRequest().setPath(location.getRoot());
			std::cout << location.getRoot() << std::endl;
			int ret_code = this->unlinkFileAndFolder(location.getRoot(), 0);
			if (ret_code == 404)
			{
				std::cerr << "opendir() error!" << std::endl;
				connection.getResponse().makeErrorResponse(400, &location);
				return (404);
			}
			connection.getResponse().makeDeleteResponse();

			return (GENERAL_RESPONSE);
		}

		// std::string path = location.getRoot() + uri.substr(location.getLocationName().length());
		switch (this->isDirectoryName(path))
		{
			case true:
			{
				if (path[path.length() - 1] != '/')
					path += '/';
				connection.getRequest().setPath(path);
				int ret_code = this->unlinkFileAndFolder(path, 1);
				if (ret_code == 404)
				{
					std::cerr << "opendir() error!" << std::endl;
					connection.getResponse().makeErrorResponse(400, &location);
					return (404);
				}
				break;
			}
			case false:
			{
				connection.getRequest().setPath(path);
				unlink(path.c_str());
				break;
			}
			case -1:
			{
				connection.getResponse().makeErrorResponse(404, &location);
				return (404);
			}
		}
		connection.getResponse().makeDeleteResponse();
	}
	else
		connection.getResponse().makeErrorResponse(501, &location);

	return (GENERAL_RESPONSE);
}

bool Webserver::isCgiRequest(Location &location, Request &request)
{ // request의 cgiinfo와 Reuqset cgi정보 비교 후 cgi 환경변수 셋팅 및 실행 및 유효성 체크
	// "locahost:8080/cgi_bin/youpi.out"

	std::map<std::string, std::string> &cgi_paths = location.getCgiPaths();
	size_t dot_pos = request.getUri().find('.'); // 14 (/cgi_bin/youpi.out)

	if (dot_pos == std::string::npos)
		return (false);

	size_t ext_end = dot_pos;

	// . 부터 / 나 ? 이전까지, 없으면 끝까지 인덱스++
	while (ext_end != request.getUri().length() && request.getUri()[ext_end] != '/' && request.getUri()[ext_end] != '?')
		ext_end++;

	std::string res = request.getUri().substr(dot_pos, ext_end - dot_pos); //요청 Uri에서 .과 / or ? 사이의 문자인 확장자 == .out

	std::map<std::string, std::string>::const_iterator found;

	//요청들어온 Cgi uri와 location cgi uri 비교
	if ((found = cgi_paths.find(res)) == cgi_paths.end())
	{
		return (false);
	}

	while (request.getUri()[dot_pos] != '/')
		dot_pos--;

	res = request.getUri().substr(dot_pos + 1, ext_end - dot_pos - 1); // youpi.out (Cgi경로에서 맨뒷부분)

	Cgi cgi;
	// (, , hi.py , /usr/bin/python)
	cgi.cgiPipeFdSet(request, location, res, found->second);
	return (true);
}

// -1 return : the path ends with /
int Webserver::createFileWithSetup(std::string path)
{
	size_t slash_idx;

	std::cout << path << std::endl;
	slash_idx = path.find("/");
	while (slash_idx != std::string::npos)
	{
		std::string dirname = path.substr(0, slash_idx);
		std::cout << dirname << std::endl;
		mkdir(dirname.c_str(), 0755);
		slash_idx = path.find("/", slash_idx + 1);
	}
	return (open(path.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0755));
}

int Webserver::isDirectoryName(const std::string &path)
{
	struct stat sb;
	if (stat(path.c_str(), &sb) == -1)
		return (-1);
	return S_ISDIR(sb.st_mode);
}

// clear away the connection(Client socket)'s info from Server, and delete and close the fd from Webserver
void Webserver::disconnect_connection(Connection &connection)
{
	if (this->getFdMap().size() == 0)
		return;

	// std::cout << "error1 " << std::endl;
	Connection *connection_pointer = &connection;
	std::vector<int> to_delete_fds;
	KqueueMonitoredFdInfo *monitor_fd = NULL;

	std::map<int, KqueueMonitoredFdInfo *> _fd_map = this->getFdMap();
	// std::cout << "error2 " << std::endl;
	for (std::map<int, KqueueMonitoredFdInfo *>::iterator iter = _fd_map.begin(); iter != _fd_map.end(); ++iter)
	{
		// std::cout << "error2-2: " << iter->first << ", " << iter->second << std::endl;
		monitor_fd = iter->second;
		if (monitor_fd->getConnection() == connection_pointer)
		{
			// std::cout << "error2-3: " << monitor_fd->getConnection()->getConnectionFd() << std::endl;
			switch (monitor_fd->getType())
			{
			// case CONNECTION_FDTYPE:
			case FILE_FDTYPE:
			case CGI_WRITE_FDTYPE:
				// std::cout << "error3 " << std::endl;
				to_delete_fds.push_back(iter->first);
				// std::cout << "error3-2 " << std::endl;
				break;
			case CGI_READ_FDTYPE:
				// std::cout << "error4 " << std::endl;
				kill(monitor_fd->getPid(), SIGKILL);
				break;
			}
		}
	}
	// std::cout << "error5 " << std::endl;
	for (std::vector<int>::const_iterator iter = to_delete_fds.begin(); iter != to_delete_fds.end(); ++iter)
	{
		unsetFdMap(*iter);
		close(*iter);
	}

	// std::cout << "error6 " << std::endl;
	int connection_fd = connection.getConnectionFd();
	// std::cout << "error7 " << std::endl;
	connection.getServer()->getConnections().erase(connection_fd);
	// std::cout << "error8 " << std::endl;
	unsetFdMap(connection_fd);
	close(connection_fd);
	// std::cout << "error9 " << std::endl;
}
int Webserver::sendResponse(Connection &connection, int monitor_event_fd)
{
	// std::cout << "write to [" << monitor_event_fd << "]\n";
	size_t res_idx = connection.getResponse().getResIdx();
	std::string uri = connection.getRequest().getUri();

	int write_size = write(monitor_event_fd, connection.getResponse().getRawResponse().c_str() + res_idx, connection.getResponse().getRawResponse().length() - res_idx);
	if (write_size == -1)
	{
		this->disconnect_connection(connection);
		std::cout << "With write error, disconnected: " << monitor_event_fd << std::endl;
		return 500;
	}
	else if (write_size >= 0)
	{
		connection.getResponse().setResIdx(res_idx + write_size);
		if (connection.getResponse().getResIdx() >= connection.getResponse().getRawResponse().length())
		{
			std::cout << "byebye~\n";
			connection.getRequest().initRequest();
			connection.getResponse().initResponse();
			connection.setStatus(REQUEST_RECEIVING);
		}

		// 에러일때도 바로 여기서 끊어주자
		if (uri == "/siege.html")
		{
			std::cout << "/siege.html" << std::endl;
			if (this->getFdMap().find(monitor_event_fd) != this->getFdMap().end())
			{
				std::map<int, KqueueMonitoredFdInfo *>::iterator iter;
				int i = 0;
				for (iter = this->fd_map.begin(); iter != this->fd_map.end(); iter++)
				{
					if (iter->second->getType() == CONNECTION_FDTYPE) // 서버 에러 - 프로그램 터짐
						i++;
					if (i > 1)
					{
						std::cout << "\nwrite disconnect = " << monitor_event_fd << std::endl;
						this->disconnect_connection(connection); // fd인스인 리소스인스와 파이프인스에 해당 Connection인스 연결되어 있으면 앞선 2개 인스를 fd_delete_fds에 입력. cgi있으면 kill pid()
						std::cout << "==================\n"
								<< std::endl;
						break;
					}
				}
			}
		}
	}
	return 0;
}

int Webserver::makePostPutResponse(KqueueMonitoredFdInfo *monitor_fd, int monitor_event_fd)
{
	size_t write_idx = monitor_fd->getWriteIdx();
	int write_size = write(monitor_event_fd, monitor_fd->getData().c_str() + write_idx, monitor_fd->getData().length() - write_idx);
	if (write_size == -1)
	{
		std::cerr << "temporary resource write error!" << std::endl;
		return 500;
	}
	else if (write_size >= 0)
	{
		monitor_fd->setWriteIdx(write_idx + write_size);
		if (monitor_fd->getWriteIdx() >= monitor_fd->getData().length())
		{
			monitor_fd->getConnection()->getResponse().makePostPutResponse();
			this->unsetFdMap(monitor_event_fd);
			close(monitor_event_fd);
		}
	}
	return 0;
}

int Webserver::writeOnPipe(KqueueMonitoredFdInfo *monitor_fd, int monitor_event_fd)
{
	/*
		(0		: child is still running)
		(-1		: wait() error)
		(else	: Success on retrieving the child process's info that is ended)
	*/
	if (waitpid(monitor_fd->getPid(), NULL, WNOHANG) != 0)
	{
		this->unsetFdMap(monitor_event_fd);
		close(monitor_event_fd);
	}
	else
	{
		int write_idx = monitor_fd->getWriteIdx();

		int write_size = write(monitor_event_fd, monitor_fd->getData().c_str() + write_idx, monitor_fd->getData().length() - write_idx);
		if (write_size == -1)
		{
			std::cerr << "temporary pipe write error!" << std::endl;
			return 500;
		}
		else if (write_size >= 0)
		{
			monitor_fd->setWriteIdx(write_idx + write_size);
			if (monitor_fd->getWriteIdx() >= monitor_fd->getData().length())
			{
				this->unsetFdMap(monitor_event_fd);
				close(monitor_event_fd);
			}
		}		
	}
	return 0;
}

int Webserver::makeUploadResponse(KqueueMonitoredFdInfo *monitor_fd, int monitor_event_fd)
{
	std::map<pid_t, std::pair<std::string, size_t> > &upload_infos = monitor_fd->getUploadFds();
	std::string upload_data = upload_infos[monitor_event_fd].first;
	size_t &write_idx = upload_infos[monitor_event_fd].second;
	if (write_idx >= upload_data.length())
		return 77777;

	int write_size = write(monitor_event_fd, upload_data.c_str() + write_idx, upload_data.length() - write_idx);
	write_idx += write_size;
	if (write_size == -1)
	{
		std::cerr << "temporary resource write error!" << std::endl;
		return 500;
	}
	else if(write_size >= 0)
	{
		bool all_uploaded = true;
		for (std::map<pid_t, std::pair<std::string, size_t> >::iterator it = upload_infos.begin(); it != upload_infos.end(); it++)
		{
			size_t upload_data_size = it->second.first.length();
			size_t writed_size = it->second.second;
			if (upload_data_size > writed_size)
			{
				all_uploaded = false;
				break;
			}
		}
		if (all_uploaded)
		{
			// std::cout << "\x1b[35m" << " 다 썻씁니다" << "\x1b[0m" << std::endl;
			std::map<int, KqueueMonitoredFdInfo *>::iterator iter = this->getFdMap().find(upload_infos.begin()->first);
			KqueueMonitoredFdInfo *to_delete_fdtype = iter->second;

			for (std::map<pid_t, std::pair<std::string, size_t> >::iterator it = upload_infos.begin(); it != upload_infos.end(); it++)
			{
				close(it->first);
				this->getFdMap().erase(it->first);
			}
			monitor_fd->getConnection()->getResponse().makeMultipartResponse("UPLOAD");
			delete to_delete_fdtype;
		}
	}
	return 0;
}

bool Webserver::execEventQueue()
{
	Webserver::getWebserverInst()->initKqueue();
	Webserver::getWebserverInst()->initServers();

	struct timespec timeout;
	timeout.tv_sec = 3;
	timeout.tv_nsec = 0;

	while (1)
	{
		int monitor_event_num = kevent(this->kq.getKqueue(), &this->kq.getChangeList()[0], this->kq.getChangeList().size(), this->kq.event_list, 1024, &timeout);
		if (monitor_event_num < 0)
		{
			std::cerr << "kevent error" << std::endl;
			continue;
		}

		this->kq.getChangeList().clear();
		for (int i = 0; i < monitor_event_num; ++i)
			this->execMonitoredEvent(&this->kq.event_list[i]);
	}
	return (true);
}

void Webserver::execMonitoredEvent(struct kevent *monitor_event)
{
	// if (this->getFdMap().find(monitor_event->ident) == this->getFdMap().end()){std::cout << "못찾았다\n";return ;} KqueueMonitorFd *selected_fd = this->getFdMap().find(selected_event->ident)->second;
	KqueueMonitoredFdInfo *monitor_fd = this->getFdMap().find(monitor_event->ident)->second;
	if (monitor_fd == NULL)
		return;

	if (monitor_event->flags & EV_ERROR)
	{
		std::cout << "is this seen-Error ?\n";
		if (monitor_fd->getType() == SERVER_FDTYPE)
			std::cerr << "Server Error!" << std::endl;
		else if (monitor_fd->getType() == CONNECTION_FDTYPE)
		{
			std::cerr << "connection error!" << std::endl;
			disconnect_connection(*(monitor_fd->getConnection()));
		}
		else if (monitor_fd->getType() == FILE_FDTYPE) // CGI_READ_FDTYPE도
		{
			std::cerr << "resource error!" << std::endl;
			monitor_fd->getConnection()->getResponse().makeErrorResponse(500, NULL);
			this->unsetFdMap(monitor_event->ident);
			close(monitor_event->ident);
		}
		else if (monitor_fd->getType() == CGI_WRITE_FDTYPE || monitor_fd->getType() == CGI_READ_FDTYPE)
		{
			std::cerr << "pipe error!" << std::endl;
			monitor_fd->getConnection()->getResponse().makeErrorResponse(500, NULL);
			this->unsetFdMap(monitor_event->ident);
			close(monitor_event->ident);
		}
		else if (monitor_fd->getType() == ERROR_FILE_FDTYPE)
		{
			std::cerr << "Error file error!" << std::endl;
			monitor_fd->getConnection()->getResponse().makeErrorResponse(500, NULL);
			this->unsetFdMap(monitor_event->ident);
			close(monitor_event->ident);
		}
		else if (monitor_fd->getType() == UPLOAD_FILE_FDTYPE)
		{
			std::map<pid_t, std::pair<std::string, size_t> > &upload_infos = monitor_fd->getUploadFds();
			std::map<int, KqueueMonitoredFdInfo *>::iterator iter = this->getFdMap().find(upload_infos.begin()->first);
			KqueueMonitoredFdInfo *to_delete_fdtype = iter->second;

			for (std::map<pid_t, std::pair<std::string, size_t> >::iterator it = upload_infos.begin(); it != upload_infos.end(); it++)
			{
				close(it->first);
				this->getFdMap().erase(it->first);
			}
			monitor_fd->getConnection()->getResponse().makeMultipartResponse("UPLOADED");
			delete to_delete_fdtype;
		}
	}
	if (monitor_event->filter == EVFILT_READ)
	{
		if (monitor_fd->getType() == SERVER_FDTYPE)
		{
			// std::cout << "\x1b[31m" << "READ - SERVER_FDTYPE"  << "\x1b[0m" << std::endl;
			this->servers_fd[monitor_event->ident]->createConnection(monitor_event->ident);
		}
		else if (monitor_fd->getType() == CONNECTION_FDTYPE)
		{
			Connection *connection = monitor_fd->getConnection();
			// if to improve RequestParse into multiple RequestsParse, the following should be made in vector like. only parse_status == HEADER would be appended to the vector. [parse_status, (method, uri, httpVersion), headers, raw_body]
			if (connection->readRequest() == DISCONNECT_CONNECTION)
			{
				std::map<int, KqueueMonitoredFdInfo *>::iterator iter;
				int i = 0;
				for (iter = this->fd_map.begin(); iter != this->fd_map.end(); iter++)
				{
					if (iter->second->getType() == CONNECTION_FDTYPE) // 서버 에러 - 프로그램 터짐
						i++;
					if (i >= 1)
					{
						std::cout << "read disconnect = " << monitor_event->ident << std::endl;
						this->disconnect_connection(*connection);
						break;
					}
				}
				return;
			}
			if (connection->getStatus() == REQUEST_COMPLETE)
			{
				std::cout << "Request Uri: " << connection->getRequest().getUri() << std::endl;
				Location &location = this->findLocation(*connection->getServer(), connection->getRequest().getUri());
				std::cout << "location: " << location.getRoot() << location.getLocationName() << std::endl;
				if (this->isValidRequestwithConfig(*connection) != 0)
				{
					std::cout << "error in isValidRequestwithConfig!" << std::endl;
					return;
				}

				if (this->isMultipart(*connection, location) == 0)
					return;
				if (this->isRedirect(*connection, location) == 0)
					return;
				else if (this->isCgi(location, connection->getRequest()))
					return;
				else if (this->isAutoIndex(*connection, location) != 0)
					return;
				else
					(this->defaultToHttpMethod(*connection, location));
				return;
			}
		}
		else if (monitor_fd->getType() == FILE_FDTYPE)
		{
			std::cout << "FILE_FDTYPE - READ" << std::endl;
			if (monitor_fd->getConnection()->getResponse().makeGetHeadResponse(monitor_event->ident, monitor_fd->getConnection()->getRequest(), monitor_event->data) == 500)
				return;
		}
		else if (monitor_fd->getType() == CGI_READ_FDTYPE)
		{
			if (monitor_fd->getConnection()->getResponse().makeCgiResponse(monitor_event->ident, monitor_fd->getConnection()->getRequest()) == 500)
				return;
		}
		else if (monitor_fd->getType() == ERROR_FILE_FDTYPE)
		{
			if (monitor_fd->getConnection()->getResponse().makeErrorFileResponse(monitor_event->ident) == 500)
				return;
		}
	}
	else if (monitor_event->filter == EVFILT_WRITE)
	{
		if (monitor_fd->getType() == CONNECTION_FDTYPE)
		{
			Connection *connection = monitor_fd->getConnection();
			if (connection->getStatus() == RESPONSE_COMPLETE)
			{
				// std::cout << "(Write Event)::: changeList- " << this->getKq().getChangeList().size() << std::endl;
				std::cout << "----->Now time to RESPONSE_COMPLETE\n";
				if (this->sendResponse(*connection, monitor_event->ident) == 500)
					return;
			}
		}
		// resource write - Only "PUT"
		else if (monitor_fd->getType() == FILE_FDTYPE)
		{
			if (this->makePostPutResponse(monitor_fd, monitor_event->ident) == 500)
				return;
		}
		else if (monitor_fd->getType() == CGI_WRITE_FDTYPE)
		{
			if (this->writeOnPipe(monitor_fd, monitor_event->ident) == 500)
				return;
		}
		else if (monitor_fd->getType() == UPLOAD_FILE_FDTYPE)
		{
			if (this->makeUploadResponse(monitor_fd, monitor_event->ident) == 500)
				return;
		}
	}
}