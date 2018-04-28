#ifndef CUTE_EPOLL_H__
#define CUTE_EPOLL_H__

#include "config.h"
#include "inc.h"

class cute_epoll_event
{
public:
	cute_epoll_event(i32 fd, u32 event)
		: fd_(fd)
		, event_(event)
	{
	}

public:
	inline i32 fd() const
	{
		return this->fd_;
	}

	inline u32 event() const
	{
		return this->event_;
	}

public:
	enum { EPOLL_IN = 0x001, EPOLL_OUT = 0x004, EPOLL_ERR = 0x008, EPOLL_HUP = 0x010};

private:
	i32 fd_;
	u32 event_;
};


class cute_epoll
{
public:
	cute_epoll();
	~cute_epoll();

public:
	i32 open();
	void close();

public:
	i32 register_socket(i32 fd);
	i32 remove_socket(i32 fd);
	i32 wait(i32 wait_for_mill_secs, std::list<cute_epoll_event> &event_list);

private:
	i32				epfd_;
	epoll_event		events_[MAX_EPOLL_EVENTS];
};



#endif


