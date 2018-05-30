#include "../lib/cute_reactor.h"
#include "../lib/cute_connector.h"
#include "../lib/cute_service_handler.h"
#include "../lib/cute_logger.h"
#include "../lib/cute_sche_timer.h"
#include "../lib/cute_mem_pool.h"
#include "../lib/cute_message.h"

#include <iomanip>

#define TEST_CLIENT
#ifdef TEST_CLIENT

#define MAX_RECV_BUF  32

cute_mem_pool* g_mem_pool_ptr = nullptr;


class reader : public data_block_reader
{
	virtual u32 execute(u8* read_buf, u32 readable_bytes)
	{
        	std::cout << "read_buf: " << std::setbase(16) << static_cast<const void*>(read_buf) << " readable_bytes: " << readable_bytes << std::endl;
                for (auto i = 0; i < readable_bytes; ++i)
                        std::cout << read_buf[i];
		std::cout << std::endl;
                return readable_bytes;
        };
};


class echo_service_handler : public cute_service_handler
{
public:
	echo_service_handler()
	: message_(g_mem_pool_ptr, MAX_RECV_BUF)
	{

	}

	virtual i32 open(const cute_socket& socket, cute_reactor* reactor)
	{
		WRITE_INFO_LOG("echo_service_handler:open, fd: " + std::to_string(socket.handle()));
		return cute_service_handler::open(socket, reactor);		
	}
	
	virtual i32 handle_input(i32 fd)
	{
		WRITE_INFO_LOG("echo_service_handler:handle_input, fd: " + std::to_string(fd));

		for(;;)
		{
			u32 byte_translated = 0;
			auto ret = this->socket_.recv(this->message_, &byte_translated);
			if (ret == CUTE_ERR) 
			{
				this->close();
				break;
			}
			else if ( byte_translated > 0)
			{
				auto reader_ptr = std::make_shared<reader>();

				/*
 				// read by data_block iterator
				std::for_each(this->message_.begin(), this->message_.end(), [&](cute_data_block& block)
				{
					block.read(reader_ptr);
				});
				*/

				// read by message
				for (;;)
				{
					u8 ch = 0;
					if (0 == this->message_.read(ch))
						break;
					std::cout << ch;
				}

				std::cout << std::endl;
				
				// reset message for reuse
				this->message_.reset();

				if (ret == CUTE_RECV_BUF_EMPTY) 	// no more data in recv buff
					break;
			}
		}

		return 0;
	}

	virtual i32 handle_output(i32 fd)
	{
		WRITE_INFO_LOG("echo_service_handler:handle_output, fd: " + std::to_string(fd));
		return 0;
	}
private:
	cute_message message_;
};


i32 main(i32 argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "usage: client <host> <port> <num>" << std::endl;
		return 0;
	}

	g_mem_pool_ptr = new cute_mem_pool();
	g_mem_pool_ptr->init(12, 4);
	cute_reactor reactor;
	reactor.init();
	auto connector = std::make_shared<cute_connector<echo_service_handler>>();
	cute_net_addr addr(argv[1], std::atoi(argv[2]));
	auto num = std::atoi(argv[3]);
	for (auto i = 0; i < num; ++i)
	connector->connect(addr, &reactor);
	reactor.run_loop();
    return 0;
}


#endif

