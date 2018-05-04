#include "cute_reactor.h"
#include "cute_socket.h"
#include "cute_socket_acceptor.h"
#include "cute_event_handler.h"
#include "cute_logger.h"
#include "util.h"

cute_reactor::cute_reactor()
{
}


cute_reactor::~cute_reactor()
{
}

i32 cute_reactor::init()
{
	WRITE_INFO_LOG("cute_reactor::init, thread: " + thread_id_helper::exec());

	// ignore SIG_PIPE broken signal
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if ((sigemptyset(&sa.sa_mask) == -1) || sigaction(SIGPIPE, &sa, 0) == -1)
		return CUTE_ERR;

	this->epoll_.open();
	this->sche_timer_.open();
	return CUTE_SUCC;
}

void cute_reactor::fini()
{
	WRITE_INFO_LOG("cute_reactor::fini");
	this->sche_timer_.close();
	this->epoll_.close();	
}

i32 cute_reactor::register_handler(std::shared_ptr<cute_event_handler> handler, const cute_socket& socket)
{
	std::lock_guard<std::mutex> guard(this->mutex_);
	WRITE_INFO_LOG("cute_reactor::register_handler, fd: " + std::to_string(socket.handle()) + " thread: " 
		+ thread_id_helper::exec());
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
	
	event_handler_proxy_ptr handler_proxy = std::make_shared<cute_event_handler_proxy>(handler);
	this->map_.insert(std::make_pair(socket.handle(), handler_proxy));
	return CUTE_SUCC;
}

i32 cute_reactor::register_handler(std::shared_ptr<cute_event_handler> handler, const cute_socket_acceptor & socket)
{
	std::lock_guard<std::mutex> guard(this->mutex_);
	WRITE_INFO_LOG("cute_reactor::register_handler, fd: " + std::to_string(socket.handle()) + " thread: "
		+ thread_id_helper::exec());
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

	event_handler_proxy_ptr handler_proxy = std::make_shared<cute_event_handler_proxy>(handler);
	this->map_.insert(std::make_pair(socket.handle(), handler_proxy));
	return CUTE_SUCC;
}

i32 cute_reactor::remove_handler(const cute_socket& socket)
{
	std::lock_guard<std::mutex> guard(this->mutex_);
	WRITE_INFO_LOG("cute_reactor::remove_handler, fd: " + std::to_string(socket.handle()) + " thread: "
		+ thread_id_helper::exec());
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
	std::lock_guard<std::mutex> guard(this->mutex_);
	WRITE_INFO_LOG("cute_reactor::remove_handler, fd: " + std::to_string(socket.handle()) + " thread: "
		+ thread_id_helper::exec());
	if (this->map_.find(socket.handle()) == this->map_.end())
	{
		WRITE_ERROR_LOG("map NOT exist socket");
		return CUTE_ERR;
	}

	this->map_.erase(socket.handle());
	this->epoll_.remove_socket(socket.handle());
	return CUTE_SUCC;
}

u64 cute_reactor::register_timer(i32 interval, i32 fd)
{
	WRITE_INFO_LOG("cute_reactor::register_timer" + thread_id_helper::exec());

	try
	{
		auto handler_proxy = this->get_handler_proxy(fd);
		auto weak_ptr = std::weak_ptr<cute_reactor::cute_event_handler_proxy>(handler_proxy);
		auto timer_handler = std::make_shared<cute_reactor_timer_handler>(weak_ptr);
		return this->sche_timer_.register_timer(interval, timer_handler);
	}
	catch (i32 fd)
	{
		WRITE_ERROR_LOG("map NOT exist socket, fd: " + std::to_string(fd) + " thread: " + thread_id_helper::exec());
		return INVALID_TIMER_ID;
	}
}

i32 cute_reactor::remove_timer(u64 timer_id)
{
	WRITE_INFO_LOG("cute_reactor::remove_timer, timer_id: " + std::to_string(timer_id) + " thread: "
		+ thread_id_helper::exec());
	return this->sche_timer_.remove_timer(timer_id);
}

void cute_reactor::run_loop()
{
	WRITE_INFO_LOG("cute_reactor::run_loop, thread: " + thread_id_helper::exec());
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
			try
			{
				event_handler_proxy_ptr handler_proxy = this->get_handler_proxy(ele.fd());
				if (ele.event() & cute_epoll_event::EPOLL_ERR || ele.event() & cute_epoll_event::EPOLL_HUP)
					handler_proxy->handle_close(ele.fd()), dispatched_cnt++;
				else if (ele.event() & cute_epoll_event::EPOLL_IN)
					handler_proxy->handle_input(ele.fd()), dispatched_cnt++;
				else if (ele.event() & cute_epoll_event::EPOLL_OUT)
					handler_proxy->handle_output(ele.fd()), dispatched_cnt++;
			}
			catch (int fd)
			{
				WRITE_ERROR_LOG("map NOT exist socket, fd: " + std::to_string(fd) + " thread: " + thread_id_helper::exec());
			}			
		});

		// WRITE_INFO_LOG("cute_reactor finish dispatch. thread: " + thread_id_helper::exec());
	}	

	// 2 sleep if dispatched_cnt equal 0
	if (0 == dispatched_cnt) 
	{
		std::chrono::milliseconds duration(REACTOR_SLEEP_INTERVAL);
		std::this_thread::sleep_for(duration);
	}	
}

std::shared_ptr<cute_reactor::cute_event_handler_proxy> cute_reactor::get_handler_proxy(i32 fd) throw(i32)
{
	std::lock_guard<std::mutex> guard(this->mutex_);
	auto it = this->map_.find(fd);
	if (it == this->map_.end())		// not found then throw exception
		throw (i32)CUTE_INVALID_FD;
	return it->second;	
}

