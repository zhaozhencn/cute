#include "cute_socket_connector.h"
#include "cute_net_addr.h"


i32 cute_socket_connector::connect(const cute_net_addr & remote_addr)
{
	i32 fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == fd)
		return CUTE_ERR;

	if (-1 == this->set_non_blocking(fd))
		return CUTE_ERR;

	sockaddr_in addr_in = remote_addr.get_sockaddr_in();
	i32 ret = ::connect(fd, (sockaddr*)&addr_in, sizeof(addr_in));
	if (0 == ret)
	{
		this->fd_ = fd;
		return CUTE_SUCC;
	}
	else if (-1 == ret && errno == EINPROGRESS)
	{
		this->fd_ = fd;
		return CUTE_CONNECT_PROCESS;
	}
	else
		return CUTE_ERR;
}


i32 cute_socket_connector::set_non_blocking(i32 fd)
{
	i32 flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return -1;

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		return -1;

	return 0;
}


