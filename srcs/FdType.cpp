#include "FdType.hpp"

FdType::FdType(t_FdType type)
{
	this->type = type;
	this->write_idx = 0;
}

FdType::FdType(t_FdType type, Connection *connection)
{
	this->type = type;
	this->connection = connection;
	this->write_idx = 0;
}

FdType::FdType(t_FdType type, Connection *connection, pid_t pid)
{
	this->type = type;
	this->connection = connection;
	this->pid = pid;
	this->write_idx = 0;
}

FdType::FdType(t_FdType type, Connection *connection, pid_t pid, std::string data)
{
	this->type = type;
	this->connection = connection;
	this->pid = pid;
	this->data = data;
	this->write_idx = 0;
}

FdType::FdType(t_FdType type, Connection *connection, std::string data)
{
	this->type = type;
	this->connection = connection;
	this->data = data;
	this->write_idx = 0;
}

FdType::FdType(t_FdType type, Connection *connection, std::vector<std::pair<std::string, std::string> > upload_file_list)
{
	this->type = type;
	this->connection = connection;
	this->upload_files = upload_file_list;
	
	for (size_t i = 0; i < upload_file_list.size(); i++)
	{
		pid_t fd = open((std::string("./upload/") + std::string("tmp__") + upload_file_list[i].first).c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0755);
		this->upload_fds[fd] = std::pair<std::string, size_t>(upload_file_list[i].second, 0);
	}
	this->write_idx = 0;
}

FdType::~FdType()
{
	return;
}

int FdType::getType()
{
	return this->type;
}

Connection *FdType::getConnection()
{
	return this->connection;
}

pid_t FdType::getPid(void)
{
	return this->pid;
}

const std::string &FdType::getData(void)
{
	return this->data;
}

size_t FdType::getWriteIdx(void)
{
	return this->write_idx;
}

void FdType::setWriteIdx(size_t write_idx)
{
	this->write_idx = write_idx;
}

std::map<pid_t, std::pair<std::string, size_t> > &FdType::getUploadFds()
{
	return this->upload_fds;
}
