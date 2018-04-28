#ifndef CUTE_NET_ADDR_H__
#define CUTE_NET_ADDR_H__

#include "inc.h"
#include "config.h"

class cute_net_addr
{
public:
	cute_net_addr(const std::string& ip, u16 port)
	{
		this->set(ip, port);
	}

	i32 set(const std::string& ip, u16 port)
	{
		if (!(port >= MIN_PORT && port <= MAX_PORT))
			return CUTE_ERR;

		this->ip_ = ip;
		this->port_ = port;
		return CUTE_SUCC;
	}

public:
	inline sockaddr_in get_sockaddr_in() const
	{
		sockaddr_in addr = { 0 };
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(this->ip_.c_str());
		addr.sin_port = htons(this->port_);
		return addr;
	}

	inline std::string ip() const {
		return this->ip_;
	}

	inline u16 port() const
	{
		return this->port_;
	}

	inline std::string str() const
	{
		return this->ip_ + ":" + std::to_string(this->port_);
	}

private:
	std::string ip_;	
	u16 port_;
};


#endif




