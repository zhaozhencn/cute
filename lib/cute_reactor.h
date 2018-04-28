#ifndef CUTE_REACTOR_H__
#define CUTE_REACTOR_H__

#include "inc.h"
#include "config.h"
#include "cute_epoll.h"
#include "cute_sche_timer.h"

class cute_event_handler;
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
	u64 register_timer(i32 interval, i32 repeat_cnt, std::shared_ptr<timer_handler> handler);
	i32 remove_timer(u64 timer_id);

public:	
	void run_loop();
	void run();

private:
	typedef std::shared_ptr<cute_event_handler> event_handler_ptr;
	typedef std::map<i32, event_handler_ptr> event_handler_map;

private:
	cute_sche_timer			sche_timer_;
	cute_epoll				epoll_;
	event_handler_map		map_;
};


#endif

