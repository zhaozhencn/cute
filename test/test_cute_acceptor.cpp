#include "../lib/config.h"
#include "../lib/cute_reactor.h"
#include "../lib/cute_acceptor.h"
#include "../lib/cute_service_handler.h"
#include "../lib/cute_logger.h"


#define TEST_CUTE_ACCEPTOR
#ifdef TEST_CUTE_ACCEPTOR

class my_service_handler : public cute_service_handler
{
public:
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

			// 输入a, 将发送缓冲区塞满，再变为“非满”， 验证触发 handle_output 
			if (content[0] == 'a')
			{
				for (auto i = 0; ; ++i)
				{
					auto to_send = std::to_string(i);
					size_t byte_translated = 0;
					i32 ret = this->socket_.send(to_send.c_str(), to_send.length(), &byte_translated);
					if (CUTE_SEND_BUF_FULL == ret)
					{
						std::cout << "CUTE_SEND_BUF_FULL" << std::endl;
						break;
					}
					else if(CUTE_ERR == ret)
					{
						std::cout << "CUTE_ERR" << std::endl;
						break;
					}
					else
						std::cout << "send: " << i << std::endl;
				}
			}
			else
				this->socket_.send(content.c_str(), content.length(), &byte_translated);			
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

	virtual i32 handle_timeout()
	{
		WRITE_INFO_LOG("my_service_handler:handle_timeout");
		return 0;
	}
};


i32 main()
{
	cute_reactor reactor;
	reactor.init();
	auto acceptor = std::make_shared<cute_acceptor<my_service_handler>>();
	cute_net_addr addr("0.0.0.0", 10000);
	acceptor->open(addr, &reactor);
	reactor.run_loop();	
    return 0;
}



#endif


