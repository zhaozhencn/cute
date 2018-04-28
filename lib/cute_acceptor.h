#ifndef CUTE_ACCEPTOR_H__
#define CUTE_ACCEPTOR_H__

#include "cute_socket_acceptor.h"
#include "cute_event_handler.h"
#include "cute_net_addr.h"
#include "cute_socket.h"
#include "cute_acceptor.h"
#include "cute_reactor.h"
#include "cute_service_handler.h"
#include "cute_logger.h"
#include "config.h"

template<typename SERVICE_HANDLER>
class cute_acceptor : public cute_event_handler
{
public:
	cute_acceptor()
	: reactor_(nullptr)
	{
	}

	~cute_acceptor()
	{

	}

public:
	i32 open(const cute_net_addr& addr, cute_reactor* reactor)
	{
		WRITE_INFO_LOG("cute_acceptor::open");

		if (CUTE_ERR == this->socket_acceptor_.open(addr))
		{
			WRITE_ERROR_LOG("socket_acceptor_.open");
			return CUTE_ERR;
		}

		if (CUTE_ERR == reactor->register_handler(shared_from_this(), this->socket_acceptor_))
		{
			WRITE_ERROR_LOG("register_handler");
			this->socket_acceptor_.close();
			return CUTE_ERR;
		}

		this->reactor_ = reactor;
		return CUTE_SUCC;
	}

public:
	virtual i32 handle_input(i32 fd)
	{
		WRITE_INFO_LOG("cute_acceptor::handle_input, fd: " + std::to_string(fd));

		for (;;)
		{
			cute_socket peer_socket;
			i32 ret = this->socket_acceptor_.accept(peer_socket);
			if (CUTE_ERR == ret)
			{
				WRITE_ERROR_LOG("socket_acceptor_.accept ret CUTE_ERR");
				return CUTE_ERR;
			}
			else if (CUTE_NO_MORE_CONNECT == ret)
			{
				WRITE_INFO_LOG("socket_acceptor_.accept ret CUTE_NO_MORE_CONNECT");
				return CUTE_ERR;
			}

			std::shared_ptr<cute_service_handler> handler = this->make_service_handler(this->reactor_);
			handler->open(peer_socket, this->reactor_);
		}

		return CUTE_SUCC;
	}


protected:
	std::shared_ptr<SERVICE_HANDLER> make_service_handler(cute_reactor* reactor)
	{
		return std::make_shared<SERVICE_HANDLER>();
	}

private:
	cute_socket_acceptor	socket_acceptor_;
	cute_reactor*			reactor_;
};


#endif

