#ifndef CUTE_SOCKET_ACCEPTOR_H__
#define CUTE_SOCKET_ACCEPTOR_H__

#include "config.h"

class cute_net_addr;
class cute_reactor;
class cute_socket;
class cute_socket_acceptor
{
public:
	i32 open(const cute_net_addr& addr);
	i32 accept(cute_socket& socket);
	void close();

public:
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


