#ifndef CUTE_EVENT_HANDLER_H__
#define CUTE_EVENT_HANDLER_H__

#include "inc.h"
#include "cute_socket.h"

class cute_event_handler : public std::enable_shared_from_this<cute_event_handler>
{
public:
	cute_event_handler();
	virtual ~cute_event_handler();

public:
	virtual i32 handle_input(i32 fd);
	virtual i32 handle_output(i32 fd);
	virtual i32 handle_timeout(u64 id);
	virtual i32 handle_close(i32 fd);
};


#endif

