#ifndef CUTE_SOCKET_H__
#define CUTE_SOCKET_H__

#include "config.h"
#include "inc.h"

class cute_socket
{
public:
	cute_socket(i32 fd = CUTE_INVALID_FD);
	~cute_socket();

public:
	i32 open(i32 fd);
	void close();

public:
	inline i32 handle() const
	{
		return this->fd_;
	}

public:
	i32 send(const char * buff, size_t len, size_t* byte_translated);
	i32 recv(char * buff, size_t len, size_t* byte_translated);
	
private:
	i32 fd_;
};


#endif


