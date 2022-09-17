#include "Location.hpp"
#include "Server.hpp"

Location::Location() : request_max_body_size(-1), auto_index(false), redirect_return(-1)
{ // location map<string,location> 리턴 할때 없으니까 map이라서 인스생성하는듯

}

Location::Location(const Location &src)
{
	this->location_name = src.location_name;
	this->root = src.root;
	this->index.assign(src.index.begin(), src.index.end());
	this->allow_methods.assign(src.allow_methods.begin(), src.allow_methods.end());
	this->error_pages.insert(src.error_pages.begin(), src.error_pages.end());
	this->request_max_body_size = src.request_max_body_size;
	this->upload_path = src.upload_path;
	this->auto_index = src.auto_index;
	this->cgi_infos = src.cgi_infos;
	this->auth_key = src.auth_key;
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
	this->request_max_body_size = src.request_max_body_size;
	this->upload_path = src.upload_path;
	this->auto_index = src.auto_index;
	this->cgi_infos.clear();
	this->cgi_infos = src.cgi_infos;
	this->auth_key = src.auth_key;
	this->redirect_return = src.redirect_return;
	this->redirect_addr = src.redirect_addr;
	return (*this);
}

void Location::setLocationName(std::string &location_name)
{ //서버인스에 map형태로 저장되어 있던 location인스의 location_name에 값 할당.
	this->location_name = location_name;
}

void Location::setRoot(const std::string &root)
{
	this->root = root;
	return;
}

void Location::setRequestMaxBodySize(int request_max_body_size)
{



	this->request_max_body_size = request_max_body_size;
	return;
}

void Location::setUploadPath(const std::string &upload_path)
{

	this->upload_path = upload_path;
	return;
}

void Location::setAutoIndex(bool auto_index)
{
	this->auto_index = auto_index;
	return;
}

void Location::setCgiInfos(std::map<std::string, std::string> &cgi_infos)
{
	this->cgi_infos = cgi_infos;
	return;
}

void Location::setAuthKey(const std::string &auth_key)
{


	this->auth_key = auth_key;
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

int Location::getRequestMaxBodySize()
{
	return (this->request_max_body_size);
}

std::map<int, std::string> &Location::getErrorPages()
{

	return (this->error_pages);
}

const std::string &Location::getUploadPath()
{
	return (this->upload_path);
}

bool Location::getAutoIndex()
{
	return (this->auto_index);
}

std::map<std::string, std::string> &Location::getCgiInfos()
{

	return (this->cgi_infos);
}

const std::string &Location::getAuthKey()
{
	return (this->auth_key);
}

int Location::getRedirectReturn()
{
	return (this->redirect_return);
}

const std::string &Location::getRedirectAddr()
{
	return (this->redirect_addr);
}