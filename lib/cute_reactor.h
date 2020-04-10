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
	template<typename T>
	i32 register_handler(std::shared_ptr<cute_event_handler> handler, const T& socket)
	{
	        std::lock_guard<std::mutex> guard(this->mutex_);
        	if (this->map_.find(socket.handle()) != this->map_.end())
                	return CUTE_ERR;
        	if (CUTE_ERR == this->epoll_.register_socket(socket.handle()))
                	return CUTE_ERR;
        	event_handler_proxy_ptr handler_proxy = std::make_shared<cute_event_handler_proxy>(handler);
	        this->map_.insert(std::make_pair(socket.handle(), handler_proxy));
	        return CUTE_SUCC;
	}

	template<typename T>
	i32 remove_handler(const T& socket)
	{
        	std::lock_guard<std::mutex> guard(this->mutex_);
	        if (this->map_.find(socket.handle()) == this->map_.end())
                	return CUTE_ERR;
	        this->map_.erase(socket.handle());
        	this->epoll_.remove_socket(socket.handle());
	        return CUTE_SUCC;
	}

public:
	u64 register_timer(i32 interval, i32 fd);
	u64 register_timer(i32 interval, std::shared_ptr<cute_event_handler> handler);
	i32 remove_timer(u64 timer_id);

public:
	i32 post(std::shared_ptr<cute_event_handler> handler, std::function<void()> func);

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

		virtual void execute() // for post execute
		{
			std::lock_guard<std::mutex> guard(this->mutex_);
		}

	private:
		std::shared_ptr<cute_event_handler>	handler_;
		std::mutex				mutex_;
	};

private:
	std::shared_ptr<cute_event_handler_proxy> get_handler_proxy(i32 fd) throw(i32);

private:
	using event_handler_proxy_ptr = std::shared_ptr<cute_event_handler_proxy>;
	using event_handler_proxy_map = std::unordered_map<i32, event_handler_proxy_ptr>;

private:
	cute_sche_timer				sche_timer_;
	cute_epoll				epoll_;
	event_handler_proxy_map			map_;
	std::mutex				mutex_;
};


#endif

