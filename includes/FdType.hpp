// FdType-> connection -> request msg->connection-> Request
// FdType-> connection -> response msg ->connection-> Response

#ifndef FDTYPE_HPP
#define FDTYPE_HPP
#include <string>
#include <map>
#include <vector>
#include <fcntl.h>
#include <unistd.h>

class Connection;

typedef enum e_status
{
	REQUEST_RECEIVING,
	REQUEST_COMPLETE,
	RESPONSE_COMPLETE,
} t_status;

typedef enum e_FDType
{
	SERVER_FDTYPE,
	CONNECTION_FDTYPE,
	FILE_FDTYPE,
	CGI_WRITE_FDTYPE,
	CGI_READ_FDTYPE,
	UPLOAD_FILE_FDTYPE,
	ERROR_FILE_FDTYPE,
} t_FdType;

// typedef enum e_rw
// {
// 	FD_RDONLY,
// 	FD_WRONLY,
// 	FD_RDWR,
// } t_rw;

////////////////////////////

// map[fd] = {file_Raw, writeidx}

class FdType
{
private:
	t_FdType type;
	Connection *connection;
	std::string data;
	size_t write_idx;
	pid_t pid;
	std::vector<std::pair<std::string, std::string> > upload_files;
	std::map<pid_t, std::pair<std::string, size_t> > upload_fds;

public:
	FdType(t_FdType type);
	FdType(t_FdType type, Connection *connection);
	FdType(t_FdType type, Connection *connection, pid_t pid);
	FdType(t_FdType type, Connection *connection, pid_t pid, std::string data);
	FdType(t_FdType type, Connection *connection, std::string data);
	FdType(t_FdType type, Connection *connection, std::vector<std::pair<std::string, std::string> > upload_files);
	~FdType();
	int getType();
	Connection *getConnection();
	pid_t getPid(void);
	const std::string &getData(void);
	size_t getWriteIdx(void);
	std::map<pid_t, std::pair<std::string, size_t> > &getUploadFds();

	void setWriteIdx(size_t write_idx);
};

// bool ft_split(const std::string &target, const std::string &sep, std::vector<std::string> &saver);
// std::string ft_itoa(int n);
// int ft_atoi_hex(const std::string &str);
// int ft_remove_directory(std::string dir);

#endif