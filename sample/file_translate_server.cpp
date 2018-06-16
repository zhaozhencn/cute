#include "../lib/config.h"
#include "../lib/cute_reactor.h"
#include "../lib/cute_acceptor.h"
#include "../lib/cute_service_handler.h"
#include "../lib/cute_logger.h"
#include "../lib/util.h"

#include <iomanip>
#include <fstream>

#define ALLOC_NODE_SIZE         256
#define ALLOC_NODE_COUNT        10
#define MAX_RECV_BUF            1460


cute_mem_pool* g_mem_pool_ptr = nullptr;

class file_writer : public data_block_reader
{
public:
	file_writer(std::ofstream& file)
	: file_(file)
	{
	}

        virtual u32 execute(u8* read_buf, u32 readable_bytes)
        {
		this->file_.write((i8*)read_buf, (i32)readable_bytes);
                return readable_bytes;
        };

private:
	std::ofstream& file_;
};


class file_translate_handler : public cute_service_handler
{
public:
	file_translate_handler()
		: recv_buf_(g_mem_pool_ptr, MAX_RECV_BUF)
		, step_(0)
		, total_recv_bytes_(0)
		, total_file_bytes_(0)
	{
		this->file_ = std::ofstream("./recv_file", std::ios::binary | std::ios::out | std::ios::trunc);
	}

	virtual i32 open(const cute_socket& socket, cute_reactor* reactor)
	{
		WRITE_INFO_LOG("file_translate_handler:open, fd: " + std::to_string(socket.handle()) 
		+ " thread: " + thread_id_helper::exec());
		return cute_service_handler::open(socket, reactor);
	}

	virtual void close()
	{
		cute_service_handler::close();
		WRITE_INFO_LOG("file_translate_handler::close");
	}

	virtual i32 handle_input(i32 fd)
	{
		WRITE_INFO_LOG("file_translate_handler:handle_input, fd: " + std::to_string(fd)
		+ " thread: " + thread_id_helper::exec());

		i32 ret = CUTE_ERR;
		if (this->step_ == 0)
			ret = this->recv_file_size();
		else
			ret = this->recv_file_content();

		if (CUTE_ERR == ret)
			this->close();
		
		return 0;	
	}

	i32 recv_file_size()
	{
		u32 byte_translated = 0;
		auto ret = this->socket_.recv(this->recv_buf_, &byte_translated);
		if (ret == CUTE_ERR)
		{
			WRITE_INFO_LOG("recv CUTE_ERROR");
			return ret;
		}
			
		auto n = this->recv_buf_.read(this->total_file_bytes_);
		if (n < sizeof(u32))
			return ret;

		this->total_recv_bytes_ += this->recv_buf_.payload_length();
		this->step_ = 1;			
		auto file_writer_ptr = std::make_shared<file_writer>(this->file_);
		std::for_each(this->recv_buf_.begin(), this->recv_buf_.end(), [&](cute_data_block& block)
		{
			block.read(file_writer_ptr);
		});

		this->recv_buf_.reset();
		return ret;
	}

	i32 recv_file_content()
	{
		i32 ret = 0;
		for (;;)
		{
			u32 byte_translated = 0;
			ret = this->socket_.recv(this->recv_buf_, &byte_translated);
			if (ret == CUTE_ERR)
	                {
			        WRITE_INFO_LOG("recv CUTE_ERROR");
				return ret;
			}

			if (byte_translated > 0)
			{
				this->total_recv_bytes_ += byte_translated;
				auto file_writer_ptr = std::make_shared<file_writer>(this->file_);
	                        std::for_each(this->recv_buf_.begin(), this->recv_buf_.end(), [&](cute_data_block& block)
        	       	        {
                        	        block.read(file_writer_ptr);
	                       	});	

	                        this->recv_buf_.reset();
			}

       	                if (ret == CUTE_RECV_BUF_EMPTY)         // no more data in recv buff
               	                break;
		}	

		if (this->total_file_bytes_ == this->total_recv_bytes_)
		{
			WRITE_INFO_LOG("************************ recv file successfully, recv bytes: " + std::to_string(this->total_recv_bytes_));
			return CUTE_SUCC;
		}

		return ret;
	}		

	virtual i32 handle_output(i32 fd)
	{
		WRITE_INFO_LOG("file_translate_handler:handle_output, fd: " + std::to_string(fd)
		+ " thread: " + thread_id_helper::exec());
		return 0;
	}
	
private:
	cute_message recv_buf_;
	i32 step_;
	u32 total_recv_bytes_;
	u32 total_file_bytes_;
	std::ofstream file_;
};


i32 main()
{
	g_mem_pool_ptr = new cute_mem_pool();
	g_mem_pool_ptr->init(ALLOC_NODE_SIZE, ALLOC_NODE_COUNT);

	cute_reactor reactor;	
	reactor.init();
	auto acceptor = std::make_shared<cute_acceptor<file_translate_handler>>();
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





