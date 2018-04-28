#include "cute_service_handler.h"
#include "cute_reactor.h"
#include "cute_logger.h"


cute_service_handler::cute_service_handler()
	: reactor_(nullptr)
{
	WRITE_INFO_LOG("cute_service_handler::cute_service_handler");
}


cute_service_handler::~cute_service_handler()
{
	WRITE_INFO_LOG("cute_service_handler::~cute_service_handler");
}

i32 cute_service_handler::handle_close(i32 fd)
{
	WRITE_INFO_LOG("cute_service_handler::handle_close, fd: " + std::to_string(fd));
	this->close();
	return 0;
}


i32 cute_service_handler::open(const cute_socket& socket, cute_reactor* reactor)
{
	WRITE_INFO_LOG("cute_service_handler::open, fd: " + std::to_string(socket.handle()));
	if (CUTE_ERR == reactor->register_handler(shared_from_this(), socket))
	{
		WRITE_ERROR_LOG("register_handler");
		return CUTE_ERR;
	}

	this->socket_ = socket;
	this->reactor_ = reactor;
	return CUTE_SUCC;
}


void cute_service_handler::close()
{
	WRITE_INFO_LOG("cute_service_handler::close, fd: " + std::to_string(this->socket_.handle()));
	if (this->reactor_ != nullptr)
		this->reactor_->remove_handler(this->socket_);
	this->socket_.close();
}


