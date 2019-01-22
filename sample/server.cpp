#include "../lib/config.h"
#include "../lib/cute_reactor.h"
#include "../lib/cute_acceptor.h"
#include "../lib/cute_service_handler.h"
#include "../lib/cute_logger.h"
#include "../lib/util.h"

#define TEST_SERVER
#ifdef TEST_SERVER

#define MAX_RECV_BUF	1024
#define TIMER_INTERVAL	5000

#define TEST_TIMER 1

class echo_service_handler : public cute_service_handler
{
public:
	echo_service_handler()
		: timer_id_(INVALID_TIMER_ID)
	{

	}

	virtual i32 open(const cute_socket& socket, cute_reactor* reactor)
	{
		WRITE_INFO_LOG("echo_service_handler:open, fd: " + std::to_string(socket.handle()) 
		+ " thread: " + thread_id_helper::exec());
		i32 ret = cute_service_handler::open(socket, reactor);
		if (CUTE_ERR == ret)
			return CUTE_ERR;

		#ifdef TEST_TIMER
		// register_timer		
		this->timer_id_ = this->reactor_->register_timer(TIMER_INTERVAL, this->socket_.handle());
		if (INVALID_TIMER_ID == this->timer_id_)
		{
			this->close();
		}
		#endif

		return 0;
	}

	virtual void close()
	{
		cute_service_handler::close();

		WRITE_INFO_LOG("echo_service_handler::close, timer_id: " + std::to_string(this->timer_id_));

		if (this->timer_id_ != INVALID_TIMER_ID)
		{
			this->reactor_->remove_timer(this->timer_id_);
			this->timer_id_ = INVALID_TIMER_ID;
		}
	}

	virtual i32 handle_input(i32 fd)
	{
		WRITE_INFO_LOG("echo_service_handler:handle_input, fd: " + std::to_string(fd)
		+ " thread: " + thread_id_helper::exec());
		return 0;
	}

	virtual i32 handle_output(i32 fd)
	{
		WRITE_INFO_LOG("echo_service_handler:handle_output, fd: " + std::to_string(fd)
		+ " thread: " + thread_id_helper::exec());
		return 0;
	}

	virtual i32 handle_timeout(u64 id)
	{
		WRITE_INFO_LOG("echo_service_handler:handle_timeout, id: " + std::to_string(id)
		+ " thread: " + thread_id_helper::exec());

		// send server time
		auto to_send = std::to_string(this->now());
		if (CUTE_ERR == this->socket_.send((u8*)to_send.c_str(), to_send.length(), nullptr)
			|| (INVALID_TIMER_ID == (this->timer_id_ = this->reactor_->register_timer(TIMER_INTERVAL, this->socket_.handle()))) )
		{
			this->close();
		}
		
		return 0;
	}

protected:
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

	std::thread t1 = std::thread([&reactor]()
	{
		reactor.run_loop();
	});

	std::thread t2 = std::thread([&reactor]()
	{
		reactor.run_loop();
	});

	t1.join();
	t2.join();

	reactor.fini();
	return 0;
}



#endif


