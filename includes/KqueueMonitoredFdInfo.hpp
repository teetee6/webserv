// KqueueMonitoredFdInfo-> connection -> request msg->connection-> Request
// KqueueMonitoredFdInfo-> connection -> response msg ->connection-> Response

#ifndef KQUEUEMONITOREDFDINFO_HPP
#define KQUEUEMONITOREDFDINFO_HPP
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
} t_KqueueMonitoredFdInfo;

class KqueueMonitoredFdInfo
{
private:
	t_KqueueMonitoredFdInfo type;
	Connection *connection;
	std::string data;
	size_t write_idx;
	pid_t pid;
	std::vector<std::pair<std::string, std::string> > upload_files;
	std::map<pid_t, std::pair<std::string, size_t> > upload_fds;

public:
	KqueueMonitoredFdInfo(t_KqueueMonitoredFdInfo type);
	KqueueMonitoredFdInfo(t_KqueueMonitoredFdInfo type, Connection *connection);
	KqueueMonitoredFdInfo(t_KqueueMonitoredFdInfo type, Connection *connection, pid_t pid);
	KqueueMonitoredFdInfo(t_KqueueMonitoredFdInfo type, Connection *connection, pid_t pid, std::string data);
	KqueueMonitoredFdInfo(t_KqueueMonitoredFdInfo type, Connection *connection, std::string data);
	KqueueMonitoredFdInfo(t_KqueueMonitoredFdInfo type, Connection *connection, std::vector<std::pair<std::string, std::string> > upload_files);
	~KqueueMonitoredFdInfo();
	int getType();
	Connection *getConnection();
	pid_t getPid(void);
	const std::string &getData(void);
	size_t getWriteIdx(void);
	std::map<pid_t, std::pair<std::string, size_t> > &getUploadFds();

	void setWriteIdx(size_t write_idx);
};

#endif