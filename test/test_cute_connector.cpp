#include "../lib/cute_reactor.h"
#include "../lib/cute_connector.h"
#include "../lib/cute_service_handler.h"
#include "../lib/cute_logger.h"
#include "../lib/cute_sche_timer.h"

#define TEST_CUTE_CONNECTOR
#ifdef TEST_CUTE_CONNECTOR

class my_service_handler : public cute_service_handler
{
public:
	my_service_handler()
		: timer_id_(INVALID_TIMER_ID)
	{

	}

	virtual i32 open(const cute_socket& socket, cute_reactor* reactor)
	{
		WRITE_INFO_LOG("my_service_handler:open, fd: " + std::to_string(socket.handle()));
		i32 ret = cute_service_handler::open(socket, reactor);
		if (CUTE_ERR == ret)
			return CUTE_ERR;
		
		// add some logic
		std::shared_ptr<cute_event_handler> self_1 = shared_from_this();
		std::shared_ptr<my_service_handler>& self = reinterpret_cast<std::shared_ptr<my_service_handler>&>(self_1);
		this->timer_id_ = this->reactor_->register_timer(5000, 5, std::make_shared<my_timer_handler>(self));

		if (INVALID_TIMER_ID == this->timer_id_)
			this->close();

		return 0;
	}

	virtual void close()
	{
		cute_service_handler::close();

		if (this->timer_id_ != INVALID_TIMER_ID)
		{
			this->reactor_->remove_timer(this->timer_id_);
			this->timer_id_ = INVALID_TIMER_ID;
		}
	}

	virtual i32 handle_input(i32 fd)
	{
		WRITE_INFO_LOG("my_service_handler:handle_input, fd: " + std::to_string(fd));

		size_t len = 1024;
		size_t byte_translated = 0;
		char buf[1024] = { 0 };
		this->socket_.recv(buf, len, &byte_translated);
		if (byte_translated > 0)
		{
			std::string content = std::string(buf, buf + byte_translated);
			std::cout << "recv: " << content << std::endl;		
		}
		else
			this->close();

		return 0;
	}

	virtual i32 handle_output(i32 fd)
	{
		WRITE_INFO_LOG("my_service_handler:handle_output, fd: " + std::to_string(fd));
		return 0;
	}

	virtual i32 handle_timeout(u64 id)
	{
		WRITE_INFO_LOG("my_service_handler:handle_timeout, id: " + std::to_string(id));

		// test send data 
		static auto data = 1;
		auto to_send = std::to_string(data);
		WRITE_INFO_LOG("my_service_handler:handle_timeout, data: " + to_send);
		this->socket_.send(to_send.c_str(), to_send.length(), nullptr);

		if (data > 4)
		{
			WRITE_INFO_LOG("it is time to exit, data: " + to_send);
			this->close();
		}

		++data;

		return 0;
	}

protected:
	class my_timer_handler : public timer_handler
	{
	public:
		my_timer_handler(std::shared_ptr<my_service_handler> service_handler)
			: service_handler_(service_handler)
		{

		}
	public:
		void exec()
		{
			this->service_handler_->handle_timeout(this->id_);
		}
	private:
		std::shared_ptr<my_service_handler> service_handler_;
	};

private:
	u64 timer_id_;
};


i32 main(i32 argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "usage: test_cute_connector <host> <port>" << std::endl;
		return 0;
	}

	cute_reactor reactor;
	reactor.init();
	auto connector = std::make_shared<cute_connector<my_service_handler>>();
	cute_net_addr addr(argv[1], std::atoi(argv[2]));
	connector->connect(addr, &reactor);
	reactor.run_loop();
    return 0;
}


#endif

