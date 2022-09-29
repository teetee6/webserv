#include "Kqueue.hpp"

Kqueue::Kqueue()
{

}

Kqueue::Kqueue(const Kqueue &src)
{
	this->kq = src.kq;
	this->change_list = src.change_list;
}

Kqueue::~Kqueue() {}

Kqueue &Kqueue::operator=(const Kqueue &src)
{
	this->kq = src.kq;
	this->change_list = src.change_list;
	return (*this);
}

int Kqueue::setKqueue(int kq)
{
	if (kq == -1)
		return -1;
	this->kq = kq;
	return 0;
}

int Kqueue::getKqueue()
{
	return this->kq;
}

std::vector<struct kevent> &Kqueue::getChangeList()
{
	return (this->change_list);
}

void Kqueue::pushChangeList(struct kevent event)
{
	this->getChangeList().push_back(event);
}

void Kqueue::createChangeListEvent(int fd, std::string mode, std::string disable)
{
	struct kevent event;

	/* EV_SET(kev, ident, filter, flags, fflags, data, udata); */
	if (mode.find("R") != std::string::npos)
	{
		if(disable == "DISABLE")
			EV_SET(&event, fd, EVFILT_READ, EV_ADD | EV_DISABLE, NULL, NULL, NULL);
		else
			EV_SET(&event, fd, EVFILT_READ, EV_ADD | EV_ENABLE, NULL, NULL, NULL);
		this->pushChangeList(event);
	}
	if (mode.find("W") != std::string::npos)
	{
		if(disable == "DISABLE")
			EV_SET(&event, fd, EVFILT_WRITE, EV_ADD | EV_DISABLE, NULL, NULL, NULL);
		else
			EV_SET(&event, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, NULL, NULL, NULL);
		this->pushChangeList(event);
	}
}
