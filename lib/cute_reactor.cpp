#include "cute_reactor.h"
#include "cute_socket.h"
#include "cute_socket_acceptor.h"
#include "cute_event_handler.h"
#include "cute_logger.h"

cute_reactor::cute_reactor()
{
}


cute_reactor::~cute_reactor()
{
}

i32 cute_reactor::init()
{
	WRITE_INFO_LOG("cute_reactor::init");
	
	// ignore SIG_PIPE broken signal
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if ((sigemptyset(&sa.sa_mask) == -1) || sigaction(SIGPIPE, &sa, 0) == -1)
		return CUTE_ERR;

	this->epoll_.open();
	return CUTE_SUCC;
}

void cute_reactor::fini()
{
	WRITE_INFO_LOG("cute_reactor::fini");
	this->epoll_.close();	
}

i32 cute_reactor::register_handler(std::shared_ptr<cute_event_handler> handler, const cute_socket& socket)
{
	WRITE_INFO_LOG("cute_reactor::register_handler, fd: " + std::to_string(socket.handle()));
	if (this->map_.find(socket.handle()) != this->map_.end())
	{
		WRITE_ERROR_LOG("map exist socket");
		return CUTE_ERR;
	}

	if (CUTE_ERR == this->epoll_.register_socket(socket.handle()))
	{
		WRITE_ERROR_LOG("epoll.register_socket");
		return CUTE_ERR;
	}

	this->map_.insert(std::make_pair(socket.handle(), handler));
	return CUTE_SUCC;
}

i32 cute_reactor::register_handler(std::shared_ptr<cute_event_handler> handler, const cute_socket_acceptor & socket)
{
	WRITE_INFO_LOG("cute_reactor::register_handler, fd: " + std::to_string(socket.handle()));
	if (this->map_.find(socket.handle()) != this->map_.end())
	{
		WRITE_ERROR_LOG("map exist socket");
		return CUTE_ERR;
	}

	if (CUTE_ERR == this->epoll_.register_socket(socket.handle()))
	{
		WRITE_ERROR_LOG("epoll.register_socket");
		return CUTE_ERR;
	}

	this->map_.insert(std::make_pair(socket.handle(), handler));
	return CUTE_SUCC;
}

i32 cute_reactor::remove_handler(const cute_socket& socket)
{
	WRITE_INFO_LOG("cute_reactor::remove_handler, fd: " + std::to_string(socket.handle()));
	if (this->map_.find(socket.handle()) == this->map_.end())
	{
		WRITE_ERROR_LOG("map NOT exist socket");
		return CUTE_ERR;
	}
				
	this->map_.erase(socket.handle());
	this->epoll_.remove_socket(socket.handle());
	return CUTE_SUCC;
}

i32 cute_reactor::remove_handler(const cute_socket_acceptor & socket)
{
	WRITE_INFO_LOG("cute_reactor::remove_handler, fd: " + std::to_string(socket.handle()));
	if (this->map_.find(socket.handle()) == this->map_.end())
	{
		WRITE_ERROR_LOG("map NOT exist socket");
		return CUTE_ERR;
	}

	this->map_.erase(socket.handle());
	this->epoll_.remove_socket(socket.handle());
	return CUTE_SUCC;
}

u64 cute_reactor::register_timer(i32 interval, i32 repeat_cnt, std::shared_ptr<timer_handler> handler)
{
	WRITE_INFO_LOG("cute_reactor::remove_handler");
	return this->sche_timer_.register_timer(interval, repeat_cnt, handler);
}

i32 cute_reactor::remove_timer(u64 timer_id)
{
	WRITE_INFO_LOG("cute_reactor::remove_timer, timer_id: " + std::to_string(timer_id));
	return this->sche_timer_.remove_timer(timer_id);
}

void cute_reactor::run_loop()
{
	WRITE_INFO_LOG("cute_reactor::run_loop");
	for (;;)
	{
		this->run();
	}
}

void cute_reactor::run()
{
	i32 dispatched_cnt = 0;

	// 1 check epoll event and dispatch it 
	std::list<cute_epoll_event> event_list;
	if (CUTE_SUCC == this->epoll_.wait(0, event_list))
	{
		std::for_each(event_list.begin(), event_list.end(), [this, &dispatched_cnt] (std::list<cute_epoll_event>::const_reference ele) {
			auto it = this->map_.find(ele.fd());
			if (it != this->map_.end())
			{
				event_handler_ptr handler = it->second;
				if (ele.event() & cute_epoll_event::EPOLL_ERR || ele.event() & cute_epoll_event::EPOLL_HUP)
					handler->handle_close(ele.fd()), dispatched_cnt++;
				else if (ele.event() & cute_epoll_event::EPOLL_IN)
					handler->handle_input(ele.fd()), dispatched_cnt++;
				else if (ele.event() & cute_epoll_event::EPOLL_OUT)
					handler->handle_output(ele.fd()), dispatched_cnt++;
			}
		});
	}

	// 2 check schedule timer and dispatch it 
	this->sche_timer_.dispatch(dispatched_cnt);

	// 3 sleep if dispatched_cnt equal 0
	if (0 == dispatched_cnt) 
	{
		std::chrono::milliseconds duration(REACTOR_SLEEP_INTERVAL);
		std::this_thread::sleep_for(duration);
	}	
}


