#ifndef CUTE_SOCKET_CONNECTOR_H__
#define CUTE_SOCKET_CONNECTOR_H__

#include "config.h"
#include "inc.h"

class cute_net_addr;
class cute_socket_connector 
{
public:
	i32 connect(const cute_net_addr& remote_addr);
	inline i32 handle() const
	{
		return this->fd_;
	}

private:
	i32 set_non_blocking(i32 fd);

private:
	i32 fd_;
};

#endif

