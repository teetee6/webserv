#include "Webserver.hpp"
#include "KqueueMonitoredFdInfo.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "ConfigParser.hpp"
#include <unistd.h>
#include <vector>

void debug()
{
	std::vector<Server> servers = Webserver::getWebserverInst()->getRealServer();
	std::cout << "=========================conf Parsing result=========================" << std::endl;
	for(std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
	{
		std::cout << "port num: " << it->getPort();
		std::cout << ",\tIP: " << it->getIP();
		std::cout << ",\tServerName: " << it->getServerName();
		std::cout << std::endl;

		std::map<std::string, Location> locs = it->getLocations();
		
		for(std::map<std::string, Location>::iterator it2 = locs.begin(); it2 != locs.end(); it2++)
		{
			std::cout << "loc path: " << it2->first;
			std::cout << std::endl;
			
			Location location = it2->second;

			for(std::list<std::string>::iterator it3 = location.getAllowMethods().begin(); it3 != location.getAllowMethods().end(); it3++)
			{
				std::cout << *it3 << " ";
			}
			std::cout << std::endl;
			std::cout << "Auth Key: " << location.getAuthKey();
			std::cout << std::endl;
			std::cout << "Auto Index: " << (location.getAutoIndex() ? "on" : "off");
			std::cout << std::endl;
			std::cout << "cgi paths-\t";
			for(std::map<std::string, std::string>::iterator it4 = location.getCgiPaths().begin(); it4 != location.getCgiPaths().end(); it4++)
			{
				std::cout << it4->first << ", " << it4->second << "\t";
			}
			std::cout << std::endl;
			std::cout << "Error pages-\t";
			for(std::map<int, std::string>::iterator it5 = location.getErrorPages().begin(); it5 != location.getErrorPages().end(); it5++)
			{
				std::cout << it5->first << ", " << it5->second << "\t";
			}
			std::cout << std::endl;
			std::cout << "Index-\t";
			for(std::list<std::string>::iterator it3 = location.getIndex().begin(); it3 != location.getIndex().end(); it3++)
			{
				std::cout << *it3 << " ";
			}
			std::cout << std::endl;
			std::cout << "Location name: " << location.getLocationName();
			std::cout << std::endl;
			std::cout << "Redirect Addr: " << location.getRedirectAddr();
			std::cout << std::endl;
			std::cout << "Redirect Return: " <<location.getRedirectReturn();
			std::cout << std::endl;
			std::cout << "bodyLimitSize: " << location.getBodyLimitSize();
			std::cout << std::endl;
			std::cout << "root: " << location.getRoot();
			std::cout << std::endl;
			std::cout << "upload Path: " << location.getUploadPath();
			std::cout << std::endl;

			std::cout << std::endl << std::endl;
		}

	}
	std::cout << "=========================end=========================" << std::endl;
}

int main(int argc, char **argv)
{
	try 
	{
		ConfigParser cf;
		if (argc == 2)
			cf.parseConfig(argv[1]);
		else if (argc == 1)
			cf.parseConfig();
		else
			throw "Argument Error";
		// debug();
		Webserver::getWebserverInst()->execEventQueue();
	} 
	catch(const char *e_msg)
	{
		std::cerr << e_msg << std::endl;
		return 1;
	}
	std::cout << "webserver closed\n";
	return 0;
}
 