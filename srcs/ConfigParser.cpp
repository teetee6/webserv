#include "ConfigParser.hpp"
#include "Webserver.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Connection.hpp"

ConfigParser::ConfigParser() {};
ConfigParser::~ConfigParser() {};

static bool inServerKeyword(const std::string &src)
{
	if (src == "listen" || src == "server_name" || src == "location")
		return (true);
	return (false);
}

static bool inLocationKeyword(const std::string &src)
{
	if (src == "error_page" || src == "allow_methods" ||
		src == "root" || src == "index" ||
		src == "auto_index" ||
		src == "body_limit_size" ||
		src == "cgi_path" || src == "return")
		return (true);
	return (false);
}

bool ConfigParser::parseConfig(const char *config_file_path)
{
	std::string line;
	std::vector<std::string> vec;
	std::string location_name;
	std::ifstream fin;

	if (std::string(config_file_path) == "")
	{
		std::cout << "\t\tOPEN DEFAULT CONFIG FILE!\n";
		fin.open("./configs/default_test.conf", std::ifstream::in);
	}
	else
		fin.open(config_file_path, std::ifstream::in);

	if (!fin.is_open())
	{
		throw "Can't open Config file";
	}
	try
	{
		const char *syntax_err_msg = "SYNTAX ERROR IN CONFIG FILE";
		bool in_server = false;
		bool in_location = false;
		bool expect_parenthesis = false;
		int in_parentheses = 0;
		int server_index = -1;
		std::string location_name = "";

		std::vector<Server> server_list;

		while (std::getline(fin, line))
		{
			if (fin.eof() && line.length() == 0)
				break;
			if (line.length() == 0)
				continue;

			// delete comments
			size_t comment_loc = line.find_first_of('#');
			if (comment_loc != std::string::npos)
				line.resize(comment_loc);

			size_t start_loc = line.find_first_not_of(" \v\n\r\t");
			if (start_loc == std::string::npos)
				line = "";
			else
			{
				line.erase(line.begin(), line.begin() + start_loc);
				size_t last_loc = line.find_last_not_of(" \v\n\r\t");
				line.erase(last_loc + 1);
			}
			
			if (line.length() == 0)
				continue;
			else
			{
				std::istringstream	iss(line);
				std::string			elem;

				while (iss >> elem)
				{
					if (expect_parenthesis && elem.compare("{") != 0)
					{
						throw syntax_err_msg;
					}
					if (elem.compare("server") == 0)
					{
						++server_index;
						server_list.resize(server_list.size() + 1, Server());
						in_server = true;
						expect_parenthesis = true;
					}
					else if(elem.compare("{") == 0)
					{
						expect_parenthesis = false;
						++in_parentheses;
					}
					else if(elem.compare("}") == 0)
					{
						--in_parentheses;
						if (in_parentheses < 0)
							throw syntax_err_msg;
						if (in_location)
							in_location = false;
						else if (in_server)
							in_server = false;
					}
					else if(expect_parenthesis == false)
					{
						if (in_location)
						{
							if (!in_server || !inLocationKeyword(elem))
							{
								throw syntax_err_msg;
							}
							if (elem.compare("error_page") == 0)
							{

								if (!(iss >> elem)) throw syntax_err_msg;
								
								char *trash;
								double err_code = std::strtod(elem.c_str() , &trash);
								if (*trash) throw syntax_err_msg;

								if (!(iss >> elem)) throw syntax_err_msg;

								server_list[server_index].getLocations()[location_name].getErrorPages()[static_cast<int>(err_code)] = elem;
							}
							else if (elem.compare("allow_methods") == 0)
							{
								while (iss >> elem)
									server_list[server_index].getLocations()[location_name].getAllowMethods().push_back(elem);
								if (server_list[server_index].getLocations()[location_name].getAllowMethods().size() == 0)
									server_list[server_index].getLocations()[location_name].getAllowMethods().push_back("GET");
							}
							else if (elem.compare("root") == 0)
							{
								if (!(iss >> elem)) throw syntax_err_msg;
								server_list[server_index].getLocations()[location_name].setRoot(elem);
							}
							else if (elem.compare("index") == 0)
							{
								while (iss >> elem)
									server_list[server_index].getLocations()[location_name].getIndex().push_back(elem);
								if (server_list[server_index].getLocations()[location_name].getIndex().size() == 0) 
									throw syntax_err_msg;
							}
							else if (elem.compare("auto_index") == 0)
							{
								if (!(iss >> elem)) throw syntax_err_msg;
								if (elem.compare("on") == 0)
									server_list[server_index].getLocations()[location_name].setAutoIndex(true);
								else
									server_list[server_index].getLocations()[location_name].setAutoIndex(false);
							}
							else if (elem.compare("body_limit_size") == 0)
							{
								if (!(iss >> elem)) throw syntax_err_msg;

								char *trash;
								double limit_size = std::strtod(elem.c_str() , &trash);
								if (*trash) throw syntax_err_msg;

								server_list[server_index].getLocations()[location_name].setBodyLimitSize(static_cast<int>(limit_size));
							}
							else if (elem.compare("cgi_path") == 0)
							{
								if (!(iss >> elem)) throw syntax_err_msg;
								std::string ext = elem;
								if (!(iss >> elem)) throw syntax_err_msg;
								std::string ext_path = elem;

								server_list[server_index].getLocations()[location_name].getCgiPaths().insert(std::make_pair(ext, ext_path));
							}
							else if (elem.compare("return") == 0)
							{
								if (!(iss >> elem)) throw syntax_err_msg;

								char *trash;
								double limit_size = std::strtod(elem.c_str() , &trash);
								if (*trash) throw syntax_err_msg;
								server_list[server_index].getLocations()[location_name].setRedirectReturn(static_cast<int>(limit_size));

								if (!(iss >> elem)) throw syntax_err_msg;
								server_list[server_index].getLocations()[location_name].setRedirectAddr(elem);
							}
						}
						else if (in_server)
						{
							if(!inServerKeyword(elem))
								throw syntax_err_msg;
							if (elem.compare("server_name") == 0)
							{	
								if (!(iss >> elem)) throw syntax_err_msg;
								server_list[server_index].setServerName(elem);
							}
							else if (elem.compare("listen") == 0)
							{	
								if (!(iss >> elem)) throw syntax_err_msg;
								
								char *trash = NULL;
								double value = std::strtod(elem.c_str(), &trash);
								if (*trash)
								{
									std::cout << trash ;
									throw syntax_err_msg;
									return 0;
								}
								server_list[server_index].setPort(static_cast<unsigned short>(value));

								if (!(iss >> elem)) throw syntax_err_msg;
								server_list[server_index].setIP(elem);
							}
							else if (elem.compare("location") == 0)
							{
								if (!(iss >> elem)) throw syntax_err_msg;

								location_name = elem;
								server_list[server_index].getLocations()[elem].setLocationName(elem);
								in_location = true;
								expect_parenthesis = true;
							}
						}
					}
					else
					{
						throw syntax_err_msg;
					}
				}
			}
			line.clear();
		}
		if (in_parentheses != 0 || expect_parenthesis)
			throw syntax_err_msg;

		std::vector<Server>& real_server = Webserver::getWebserverInst()->getRealServer();
		std::set<unsigned short> duplicate_port;
		for(size_t i=0; i< server_list.size(); i++)
		{
			unsigned short port_num = server_list[i].getPort();
			if (duplicate_port.find(port_num) != duplicate_port.end())
				throw "Duplicate Port Error";
			duplicate_port.insert(port_num);
			real_server.push_back(server_list[i]);
		}
	}
	catch (const char *e)
	{
		fin.close();
		throw e;
	}
	fin.close();
	return (true);
}