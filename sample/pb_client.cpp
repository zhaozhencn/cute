#include "../lib/cute_reactor.h"
#include "../lib/cute_connector.h"
#include "../lib/cute_service_handler.h"
#include "../lib/cute_logger.h"
#include "../lib/cute_sche_timer.h"
#include "../lib/cute_mem_pool.h"
#include "../lib/cute_message.h"
#include <iomanip>
#include "./address.pb.h"

#define MAX_POOL_NODE_SIZE 	20
#define MAX_POOL_NODE_CNT 	10
#define MAX_RECV_BUF  		40

cute_mem_pool* g_mem_pool_ptr = nullptr;

// decode data by protobuf
class decode 
{
public:
	enum { HEAD_SIZE = 4 };

public:
	static i32 execute(cute_message& message)
	{
		u32 proto_len = 0;
		for (; u32_size == message.peek(proto_len) && HEAD_SIZE + proto_len <= message.payload_length(); )
		{
			std::string proto(proto_len, 0);
			u8* buf = (u8*)proto.c_str();
			message.read(proto_len);	// read head
			message.read(buf, proto_len);	// read content
			tutorial::AddressBook book;
			std::cout << buf << std::endl;
			bool ret = book.ParseFromArray(buf, proto_len);
			std::cout << "parse ret: " << ret << "  proto_len: " << proto_len << std::endl;
			if (ret)
			{ 
				auto&& k = book.persion();
				for(auto it = k.begin(); it != k.end(); ++it)
				{
					std::cout << it->name() << std::endl;
					std::cout << it->age() << std::endl;
				}
			}
		}	

		return 0;
	}
};

// service handler per connection 
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

			if ( byte_translated > 0)
				decode::execute(this->message_);		

			if (ret == CUTE_RECV_BUF_EMPTY) 	// no more data in recv buff
				break;
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
	if (argc < 4)
	{
		std::cout << "usage: client <host> <port> <conn_num>" << std::endl;
		return 0;
	}

	// init mem pool 
	g_mem_pool_ptr = new cute_mem_pool();
	g_mem_pool_ptr->init(MAX_POOL_NODE_SIZE, MAX_POOL_NODE_CNT);

	// init reactor and launcher it
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



