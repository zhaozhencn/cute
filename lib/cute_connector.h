#ifndef CUTE_CONNECTOR_H__
#define CUTE_CONNECTOR_H__

#include "cute_event_handler.h"
#include "cute_net_addr.h"
#include "cute_service_handler.h"
#include "cute_socket_connector.h"
#include "cute_reactor.h"
#include "cute_logger.h"
#include "config.h"

template<typename SERVICE_HANDLER>
class cute_connector : public cute_event_handler
{
public:
	cute_connector()
		: reactor_(nullptr)
	{
	}

	~cute_connector()
	{

	}

public:
	i32 connect(const cute_net_addr& remote_addr, cute_reactor* reactor)
	{
		WRITE_INFO_LOG("cute_connector::connect");
		cute_socket_connector connector;
		i32 ret = connector.connect(remote_addr);		// non-blocked connect
		if (CUTE_ERR == ret)
		{
			WRITE_ERROR_LOG("connector.connect" + remote_addr.str());
			return CUTE_ERR;
		}
		else if (CUTE_SUCC == ret)						// if connect completed immediately
		{
			WRITE_INFO_LOG("connect completed immediately");
			std::shared_ptr<cute_service_handler> handler = this->make_service_handler(reactor);
			cute_socket socket(connector.handle());
			if (CUTE_ERR == handler->open(socket, reactor))
			{
				WRITE_ERROR_LOG("handler->open");
				socket.close();
				return CUTE_ERR;
			}
		}
		else if (CUTE_CONNECT_PROCESS == ret)			// if connect is processing
		{
			WRITE_INFO_LOG("connect is processing");
			cute_socket socket(connector.handle());
			if (CUTE_ERR == reactor->register_handler(shared_from_this(), socket))
			{
				WRITE_ERROR_LOG("reactor->register_handler");
				socket.close();
				return CUTE_ERR;
			}

			this->reactor_ = reactor;
		}

		return CUTE_SUCC;
	}

	virtual i32 handle_output(i32 fd)
	{
		WRITE_INFO_LOG("cute_connector::handle_output, fd: " + std::to_string(fd));

		cute_socket socket(fd);
		this->reactor_->remove_handler(socket);			// remove handler associate with connector from reactor first

		i32 error = 0;
		socklen_t length = sizeof(error);
		if (::getsockopt(socket.handle(), SOL_SOCKET, SO_ERROR, &error, &length) < 0)
		{
			WRITE_ERROR_LOG("getsockopt SOL_SOCKET SO_ERROR");
			socket.close();
			return CUTE_ERR;
		}

		std::shared_ptr<cute_service_handler> handler = this->make_service_handler(this->reactor_);
		if (CUTE_ERR == handler->open(socket, this->reactor_))
		{
			WRITE_ERROR_LOG("handler->open");
			socket.close();
			return CUTE_ERR;
		}

		return CUTE_SUCC;
	}

protected:
	std::shared_ptr<SERVICE_HANDLER> make_service_handler(cute_reactor* reactor)
	{
		return std::make_shared<SERVICE_HANDLER>();
	}

private:
	cute_reactor*	reactor_;
};


#endif

