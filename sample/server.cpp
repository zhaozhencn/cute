#include "../lib/config.h"
#include "../lib/cute_reactor.h"
#include "../lib/cute_acceptor.h"
#include "../lib/cute_service_handler.h"
#include "../lib/cute_logger.h"


#define TEST_SERVER
#ifdef TEST_SERVER

#define MAX_RECV_BUF	1024
#define TIMER_INTERVAL	1000
#define TIMER_REPEAT	-1			// infinite times

class echo_service_handler : public cute_service_handler
{
public:
	echo_service_handler()
		: timer_id_(INVALID_TIMER_ID)
	{

	}

	virtual i32 open(const cute_socket& socket, cute_reactor* reactor)
	{
		WRITE_INFO_LOG("echo_service_handler:open, fd: " + std::to_string(socket.handle()));
		i32 ret = cute_service_handler::open(socket, reactor);
		if (CUTE_ERR == ret)
			return CUTE_ERR;

		// register_timer
		std::shared_ptr<cute_event_handler> me = shared_from_this();
		std::shared_ptr<echo_service_handler>& self = reinterpret_cast<std::shared_ptr<echo_service_handler>&>(me);
		this->timer_id_ = this->reactor_->register_timer(TIMER_INTERVAL, TIMER_REPEAT, std::make_shared<my_timer_handler>(self));
		if (INVALID_TIMER_ID == this->timer_id_)
		{
			this->close();
		}

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
		WRITE_INFO_LOG("echo_service_handler:handle_input, fd: " + std::to_string(fd));

		size_t len = MAX_MSS_LEN;
		size_t byte_translated = 0;
		char buf[MAX_MSS_LEN] = { 0 };

		this->socket_.recv(buf, len, &byte_translated);
		if (byte_translated > 0)
		{
			std::string content = std::string(buf, buf + byte_translated);
			std::cout << "recv: " << content << std::endl;
			this->socket_.send(content.c_str(), content.length(), &byte_translated);			
		}
		else 
		{
			this->close();		// peer is closed
		}

		return 0;
	}

	virtual i32 handle_output(i32 fd)
	{
		WRITE_INFO_LOG("echo_service_handler:handle_output, fd: " + std::to_string(fd));
		return 0;
	}

	virtual i32 handle_timeout(u64 id)
	{
		WRITE_INFO_LOG("echo_service_handler:handle_timeout, id: " + std::to_string(id));

		// send server time
		auto to_send = std::to_string(this->now());
		if (CUTE_ERR == this->socket_.send(to_send.c_str(), to_send.length(), nullptr))
			this->close();

		return 0;
	}

protected:
	class my_timer_handler : public timer_handler
	{
	public:
		my_timer_handler(std::shared_ptr<echo_service_handler> service_handler)
			: service_handler_(service_handler)
		{

		}
	public:
		void exec()
		{
			this->service_handler_->handle_timeout(this->id_);
		}
	private:
		std::shared_ptr<echo_service_handler> service_handler_;
	};

	u64 now()
	{
		using namespace std::chrono;
		auto now = high_resolution_clock::now();
		return duration_cast<seconds>(now.time_since_epoch()).count();
	}

private:
	u64 timer_id_;
};


i32 main()
{
	cute_reactor reactor;
	reactor.init();
	auto acceptor = std::make_shared<cute_acceptor<echo_service_handler>>();
	cute_net_addr addr("0.0.0.0", 11000);
	acceptor->open(addr, &reactor);
	reactor.run_loop();	
    return 0;
}



#endif


