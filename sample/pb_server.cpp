#include "../lib/config.h"
#include "../lib/cute_reactor.h"
#include "../lib/cute_acceptor.h"
#include "../lib/cute_service_handler.h"
#include "../lib/cute_logger.h"
#include "../lib/util.h"
#include "../lib/cute_message.h"
#include "./address.pb.h"
#include <sstream>


#define MAX_POOL_NODE_SIZE 	20
#define MAX_POOL_NODE_CNT 	10
#define MAX_RECV_BUF  		40
#define TIMER_INTERVAL		1000
#define TEST_TIMER 		1

cute_mem_pool* g_mem_pool_ptr = nullptr;

// service handler by connection
class echo_service_handler : public cute_service_handler
{
public:
	echo_service_handler()
		: timer_id_(INVALID_TIMER_ID)
		, send_buf_(g_mem_pool_ptr, MAX_RECV_BUF)
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
		// auto to_send = std::to_string(this->now());

		tutorial::AddressBook address_book;
	        auto* p = address_book.add_persion();
        	p->set_name("name");
		static int age = 18;
	        p->set_age(age++);
        	std::stringstream ss;
	        address_book.SerializeToOstream(&ss);

		auto&& ref = ss.str();
		this->send_buf_.write((u32)ref.length());	// write data to buf
		this->send_buf_.write((u8*)ref.c_str(), ref.length());

		u32 byte_translated = 0;
                i32 ret = this->socket_.send(this->send_buf_, &byte_translated);		
		if (CUTE_ERR == ret || (INVALID_TIMER_ID == (this->timer_id_ = this->reactor_->register_timer(TIMER_INTERVAL, this->socket_.handle()))) )
		{
			this->close();
		}

		this->send_buf_.reset(); // reset for reuse 
		
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

private:
	cute_message send_buf_;
};


i32 main()
{
	// init mem pool
	g_mem_pool_ptr = new cute_mem_pool();
	g_mem_pool_ptr->init(MAX_POOL_NODE_SIZE, MAX_POOL_NODE_CNT);

	// init reactor and launcher it
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





