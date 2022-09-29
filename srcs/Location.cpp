#include "Location.hpp"
#include "Server.hpp"

Location::Location() : body_limit_size(-1), auto_index(false), redirect_return(-1)
{ 
}

Location::Location(const Location &src)
{
	this->location_name = src.location_name;
	this->root = src.root;
	this->index.assign(src.index.begin(), src.index.end());
	this->allow_methods.assign(src.allow_methods.begin(), src.allow_methods.end());
	this->error_pages.insert(src.error_pages.begin(), src.error_pages.end());
	this->body_limit_size = src.body_limit_size;
	this->auto_index = src.auto_index;
	this->cgi_paths = src.cgi_paths;
	this->redirect_return = src.redirect_return;
	this->redirect_addr = src.redirect_addr;
}

Location::~Location() {}

Location &Location::operator=(const Location &src)
{
	this->location_name = src.location_name;
	this->root = src.root;
	this->index.assign(src.index.begin(), src.index.end());
	this->allow_methods.assign(src.allow_methods.begin(), src.allow_methods.end());
	this->error_pages.insert(src.error_pages.begin(), src.error_pages.end());
	this->body_limit_size = src.body_limit_size;
	this->auto_index = src.auto_index;
	this->cgi_paths.clear();
	this->cgi_paths = src.cgi_paths;
	this->redirect_return = src.redirect_return;
	this->redirect_addr = src.redirect_addr;
	return (*this);
}

void Location::setLocationName(std::string &location_name)
{
	this->location_name = location_name;
}

void Location::setRoot(const std::string &root)
{
	this->root = root;
	return;
}

void Location::setBodyLimitSize(int body_limit_size)
{
	this->body_limit_size = body_limit_size;
	return;
}

void Location::setAutoIndex(bool auto_index)
{
	this->auto_index = auto_index;
	return;
}

void Location::setCgiPaths(std::map<std::string, std::string> &cgi_paths)
{
	this->cgi_paths = cgi_paths;
	return;
}

void Location::setRedirectReturn(int redirect_return)
{
	this->redirect_return = redirect_return;
	return;
}

void Location::setRedirectAddr(const std::string &redirect_addr)
{
	this->redirect_addr = redirect_addr;
	return;
}

const std::string &Location::getLocationName(void)
{
	return (this->location_name);
}

const std::string &Location::getRoot()
{
	return (this->root);
}

std::list<std::string> &Location::getIndex()
{
	return (this->index);
}

std::list<std::string> &Location::getAllowMethods()
{
	return (this->allow_methods);
}

int Location::getBodyLimitSize()
{
	return (this->body_limit_size);
}

std::map<int, std::string> &Location::getErrorPages()
{

	return (this->error_pages);
}

bool Location::getAutoIndex()
{
	return (this->auto_index);
}

std::map<std::string, std::string> &Location::getCgiPaths()
{
	return (this->cgi_paths);
}

int Location::getRedirectReturn()
{
	return (this->redirect_return);
}

const std::string &Location::getRedirectAddr()
{
	return (this->redirect_addr);
}