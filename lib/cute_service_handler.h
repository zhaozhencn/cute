#ifndef CUTE_SERVICE_HANDLER_H__
#define CUTE_SERVICE_HANDLER_H__

#include "cute_event_handler.h"
#include "cute_socket.h"

class cute_reactor;
class cute_service_handler : public cute_event_handler
{
public:
	cute_service_handler();
	virtual ~cute_service_handler();

public:
	virtual i32 handle_close(i32 fd);

public:
	virtual i32 open(const cute_socket& socket, cute_reactor* reactor);
	virtual void close();

protected:
	cute_socket	socket_;
	cute_reactor*	reactor_;
};


#endif

