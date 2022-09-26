#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#include <list>
#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <sys/event.h>

#include "KqueueMonitoredFdInfo.hpp"

class Kqueue
{
private:
	int kq;
	std::vector<struct kevent> change_list;

public:
	struct kevent event_list[1024];
	Kqueue();
	Kqueue(const Kqueue &src);
	virtual ~Kqueue();
	Kqueue &operator=(const Kqueue &src);

	int setKqueue(int kq);
	int getKqueue();
	void createChangeListEvent(int fd, std::string mode, std::string disable = "ABLE");
	std::vector<struct kevent> &getChangeList();
	void pushChangeList(struct kevent event);
};

#endif

