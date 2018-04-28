#include "../lib/cute_reactor.h"
#include "../lib/cute_connector.h"
#include "../lib/cute_service_handler.h"
#include "../lib/cute_logger.h"
#include "../lib/cute_sche_timer.h"

#define TEST_CLIENT
#ifdef TEST_CLIENT

#define MAX_RECV_BUF  1024

class echo_service_handler : public cute_service_handler
{
public:
	echo_service_handler()
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
		WRITE_INFO_LOG("echo_service_handler:handle_output, fd: " + std::to_string(fd));
		return 0;
	}
};


i32 main(i32 argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "usage: client <host> <port>" << std::endl;
		return 0;
	}

	cute_reactor reactor;
	reactor.init();
	auto connector = std::make_shared<cute_connector<echo_service_handler>>();
	cute_net_addr addr(argv[1], std::atoi(argv[2]));
	connector->connect(addr, &reactor);
	reactor.run_loop();
    return 0;
}


#endif

