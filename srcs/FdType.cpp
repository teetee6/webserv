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

// ServerFD::ServerFD(t_FdType type)
// {


// 	this->type = type;
// }
// ServerFD::~ServerFD() {}

// int FdType::getType()
// {
// 	return (this->type);
// }

// ConnectionFd::ConnectionFd(t_FdType type, Connection *connection)
// {

// 	this->type = type;
// 	this->connection = connection;
// }

// ConnectionFd::~ConnectionFd() {}

// Connection *ConnectionFd::getConnection()
// {
// 	return this->connection;
// }

// ResourceFD::ResourceFD(t_FdType type, Connection *connection)
// {

// 	this->type = type;
// 	this->connection = connection;
// 	this->data = NULL;
// 	this->write_idx = 0;
// }

// ResourceFD::ResourceFD(t_FdType type, Connection *connection, const std::string &data)
// {

// 	this->type = type;
// 	this->connection = connection;
// 	this->write_idx = 0;
// 	this->data = &data;
// }

// ResourceFD::ResourceFD(t_FdType type, pid_t pid, Connection *connection)
// {

// 	this->type = type;
// 	this->pid = pid;
// 	this->connection = connection;
// 	this->data = NULL;
// 	this->write_idx = 0;
// }

// ResourceFD::~ResourceFD() {}

// Connection *ResourceFD::getConnection(void)
// {
// 	return (this->connection);
// }

// pid_t ResourceFD::getPid(void)
// {
// 	return (this->pid);
// }

// const std::string &ResourceFD::getData()
// {
// 	return (*this->data);
// }

// size_t ResourceFD::getWriteIdx()
// {
// 	return (this->write_idx);
// }

// void ResourceFD::setWriteIdx(size_t write_idx)
// {
// 	this->write_idx = write_idx;
// }

// PipeFd::PipeFd(t_FdType type, pid_t pid, Connection *connection, const std::string &data) : data(data)
// {
// 	this->type = type;
// 	this->pid = pid;
// 	this->connection = connection;
// 	this->write_idx = 0;
// }

// PipeFd::~PipeFd() {}

// Connection *PipeFd::getConnection(void)
// {
// 	return (this->connection);
// }

// pid_t PipeFd::getPid(void)
// {
// 	return (this->pid);
// }

// const std::string &PipeFd::getData()
// {
// 	return (this->data);
// }

// size_t PipeFd::getWriteIdx()
// {
// 	return (this->write_idx);
// }

// void PipeFd::setWriteIdx(size_t write_idx)
// {
// 	this->write_idx = write_idx;
// }
