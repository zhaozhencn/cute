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
#define TEST_SEND_BUFF_FULL 1

cute_mem_pool* g_mem_pool_ptr = nullptr;

class echo_service_handler : public cute_service_handler
{
public:
	echo_service_handler()
		: timer_id_(INVALID_TIMER_ID)
		, message_(g_mem_pool_ptr, MAX_RECV_BUF)
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

		#ifdef TEST_SEND_BUFF_FULL
		for (auto i = 0; i < 10000; ++i)
		{
			auto m = cute_message(g_mem_pool_ptr, MAX_RECV_BUF);
			while (!m.is_write_full())
				m.write(i % 26 + 'A');
			this->queue_.push(std::move(m));		
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

		for (;;)
		{
			u32 byte_translated = 0;
			auto ret = this->socket_.recv(this->message_, &byte_translated);
			if (ret == CUTE_ERR)
			{
				WRITE_INFO_LOG("recv CUTE_ERROR");
				this->close();
				break;
			}
			else 
			{
				if (this->message_.is_write_full())
				{
					WRITE_INFO_LOG("this->message_.is_write_full");
					this->queue_.push(std::move(this->message_));
					this->message_ = cute_message(g_mem_pool_ptr, MAX_RECV_BUF);

#ifdef TEST_SEND_BUFF_FULL
					this->reply_message();
#endif
				}

				if (ret == CUTE_RECV_BUF_EMPTY) 	// no more data in recv buff
					break;
			}
		}

		return 0;
	}

	virtual i32 handle_output(i32 fd)
	{
		WRITE_INFO_LOG("echo_service_handler:handle_output, fd: " + std::to_string(fd)
		+ " thread: " + thread_id_helper::exec());

#ifdef TEST_SEND_BUFF_FULL
		this->reply_message();
#endif

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

	void reply_message()
	{
		WRITE_INFO_LOG("reply_message");
		if (this->queue_.empty())
			return;	
		auto&& message = this->queue_.front();
		u32 byte_translated = 0;
		i32 ret = CUTE_SUCC;
		for (;;)
		{
			ret = this->socket_.send(message, &byte_translated);
			if (CUTE_ERR == ret)
			{
				this->close();
				break;
			}
			else 
			{
                                if (CUTE_SEND_BUF_FULL == ret)          // send buff is full
					WRITE_INFO_LOG("CUTE_SEND_BUF_FULL...........");

				if (message.payload_length() == 0)	// curr message has been finished
				{
					WRITE_INFO_LOG("message.payload_length() == 0 and queue pop");
					this->queue_.pop();
				}				

				if (CUTE_SEND_BUF_FULL == ret || message.payload_length() == 0)
					break;
			}
		}

		// try send remain message if previous operation has been completed succ
		if (ret == CUTE_SUCC)
			reply_message();
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
	std::queue<cute_message> queue_;
	cute_message message_;
	bool is_sending_;
};


i32 main()
{
	g_mem_pool_ptr = new cute_mem_pool();
	g_mem_pool_ptr->init(12, 4);

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

    return 0;
}



#endif


