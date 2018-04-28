#include "cute_socket_acceptor.h"
#include "config.h"
#include "cute_socket.h"
#include "cute_net_addr.h"

i32 cute_socket_acceptor::open(const cute_net_addr & addr)
{
	i32 fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == fd)
		return CUTE_ERR;	

	sockaddr_in addr_in = addr.get_sockaddr_in();
	if (0 == bind(fd, (sockaddr*)&addr_in, sizeof(sockaddr)) 
		&& 0 == this->set_non_blocking(fd)
		&& 0 == listen(fd, SOMAXCONN))
	{
		this->fd_ = fd;
		return CUTE_SUCC;
	}

	if (fd != CUTE_INVALID_FD)
		::close(fd);
	return CUTE_ERR;
}

i32 cute_socket_acceptor::accept(cute_socket& socket)
{
	sockaddr_in addr_in = { 0 };
	socklen_t len = sizeof(addr_in);
	i32 conn_fd = ::accept(this->fd_, (struct sockaddr*)&addr_in, &len);
	if (-1 == conn_fd)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return CUTE_NO_MORE_CONNECT;
		else
			return CUTE_ERR;
	}
	
	socket.open(conn_fd);
	if (-1 == this->set_non_blocking(conn_fd))
	{
		socket.close();
		return CUTE_ERR;
	}
		
	return CUTE_SUCC;
}

void cute_socket_acceptor::close()
{
	if (this->fd_ != CUTE_INVALID_FD)
		::close(this->fd_);
	this->fd_ = CUTE_INVALID_FD;
}

i32 cute_socket_acceptor::set_non_blocking(i32 fd) {
	i32 flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) 
		return -1;

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) 
		return -1;

	return 0;
}




