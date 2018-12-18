#ifndef CUTE_REACTOR_H__
#define CUTE_REACTOR_H__

#include "inc.h"
#include "config.h"
#include "cute_epoll.h"
#include "cute_sche_timer.h"
#include "cute_event_handler.h"

class cute_socket;
class cute_socket_acceptor;

class cute_reactor
{
public:
	cute_reactor();
	~cute_reactor();

public:
	i32 init();
	void fini();

public:
	i32 register_handler(std::shared_ptr<cute_event_handler> handler, const cute_socket& socket);
	i32 register_handler(std::shared_ptr<cute_event_handler> handler, const cute_socket_acceptor& socket);
	i32 remove_handler(const cute_socket& socket);
	i32 remove_handler(const cute_socket_acceptor& socket);

public:
	u64 register_timer(i32 interval, i32 fd);
	u64 register_timer(i32 interval, std::shared_ptr<cute_event_handler> handler);
	i32 remove_timer(u64 timer_id);

public:	
	void run_loop();
	void run();

public:
	// handler proxy for both cute_event_handler and timer_handler
	class cute_event_handler_proxy : public cute_event_handler, public timer_handler
	{
	public:
		cute_event_handler_proxy(std::shared_ptr<cute_event_handler> handler)
			: handler_(handler)
		{

		}

	public:
		virtual i32 handle_input(i32 fd)
		{
			std::lock_guard<std::mutex> guard(this->mutex_);
			return this->handler_->handle_input(fd);
		}

		virtual i32 handle_output(i32 fd)
		{
			std::lock_guard<std::mutex> guard(this->mutex_);
			return this->handler_->handle_output(fd);
		}

		virtual i32 handle_timeout(u64 id)
		{
			std::lock_guard<std::mutex> guard(this->mutex_);
			return this->handler_->handle_timeout(id);
		}

		virtual i32 handle_close(i32 fd)
		{
			std::lock_guard<std::mutex> guard(this->mutex_);
			return this->handler_->handle_close(fd);
		}

	public:
		virtual i32 exec()  // for sche_timer
                {
			return this->handle_timeout(this->id_);
		}

	private:
		std::shared_ptr<cute_event_handler>	handler_;
		std::mutex				mutex_;
	};

private:
	std::shared_ptr<cute_event_handler_proxy> get_handler_proxy(i32 fd) throw(i32);

private:
	typedef std::shared_ptr<cute_event_handler_proxy> event_handler_proxy_ptr;
	typedef std::map<i32, event_handler_proxy_ptr> event_handler_proxy_map;

private:
	cute_sche_timer				sche_timer_;
	cute_epoll				epoll_;
	event_handler_proxy_map			map_;
	std::mutex				mutex_;
};


#endif

