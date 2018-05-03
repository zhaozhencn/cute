#include "cute_epoll.h"


cute_epoll::cute_epoll()
	: epfd_(CUTE_INVALID_FD)
{
}

cute_epoll::~cute_epoll()
{
	close();
}

i32 cute_epoll::open()
{
	if (CUTE_INVALID_FD != this->epfd_)
		return CUTE_ERR;

	this->epfd_ = ::epoll_create1(0);
	return -1 == this->epfd_ ? CUTE_ERR : CUTE_SUCC;
}

void cute_epoll::close()
{
	if (CUTE_INVALID_FD == this->epfd_)
		return;

	::close(this->epfd_);
	this->epfd_ = CUTE_INVALID_FD;
}

i32 cute_epoll::register_socket(i32 fd)
{
	std::lock_guard<std::mutex> guard(this->mutex_);
	if (CUTE_INVALID_FD == this->epfd_)
		return CUTE_ERR;

	epoll_event event = { 0 };
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLOUT | EPOLLET;
	i32 ret = ::epoll_ctl(this->epfd_, EPOLL_CTL_ADD, fd, &event);
	return -1 == ret ? CUTE_ERR : CUTE_SUCC;
}

i32 cute_epoll::remove_socket(i32 fd)
{
	std::lock_guard<std::mutex> guard(this->mutex_);
	if (CUTE_INVALID_FD == this->epfd_)
		return CUTE_ERR;

	epoll_event event = { 0 };
	event.data.fd = fd;
	i32 ret = ::epoll_ctl(this->epfd_, EPOLL_CTL_DEL, fd, &event);
	return -1 == ret ? CUTE_ERR : CUTE_SUCC;
}

i32 cute_epoll::wait(i32 wait_for_mill_secs, std::list<cute_epoll_event>& event_list)
{
	std::lock_guard<std::mutex> guard(this->mutex_);
	if (CUTE_INVALID_FD == this->epfd_)
		return CUTE_ERR;

	memset(this->events_, 0, sizeof(events_));
	i32 ret = ::epoll_wait(this->epfd_, this->events_, MAX_EPOLL_EVENTS, wait_for_mill_secs);
	if (ret > 0)
	{
		for (auto i = 0; i < ret; ++i)
			event_list.push_back(cute_epoll_event(this->events_[i].data.fd, this->events_[i].events));
	}

	return ret >= 0 ? CUTE_SUCC : CUTE_ERR;
}



