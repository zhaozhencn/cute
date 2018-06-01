#include "../lib/cute_reactor.h"
#include "../lib/cute_connector.h"
#include "../lib/cute_service_handler.h"
#include "../lib/cute_logger.h"
#include "../lib/cute_sche_timer.h"
#include "../lib/cute_mem_pool.h"
#include "../lib/cute_message.h"

#include <iomanip>
#include <fstream>

#define ALLOC_NODE_SIZE		256
#define ALLOC_NODE_COUNT	10
#define MAX_RECV_BUF  		1460

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



class file_reader : public data_block_writer
{
public:
        file_reader(std::ifstream& file)
        : file_(file)
        , ret_(CUTE_SUCC)
        {
        }

        virtual u32 execute(u8* write_buf, u32 writeable_bytes) override
        {
                this->ret_ = this->file_.readsome((i8*)write_buf, (i32)writeable_bytes);
                return this->ret_ < 0 ? 0 : this->ret_;
        }

        inline i32 ret() const
        {
                return this->ret_;
        }

private:
        std::ifstream& file_;
        i32 ret_;
};


class file_translate_service_handler;
class file_cute_connector : public cute_connector<file_translate_service_handler>
{
public:
	file_cute_connector(const std::string& file_path)
	: file_path_(file_path)
	{
	}

protected:
	std::shared_ptr<file_translate_service_handler> make_service_handler(cute_reactor* reactor)
        {
                return std::make_shared<file_translate_service_handler>(this->file_path_);
        }

private:
	std::string file_path_;
};
	

class file_translate_service_handler : public cute_service_handler
{
public:
	file_translate_service_handler()
	: recv_buf_(g_mem_pool_ptr, MAX_RECV_BUF)
        , send_buf_(g_mem_pool_ptr, MAX_RECV_BUF)
        , total_sent_bytes_(0)
        , total_file_bytes_(0)
	{
		// exist for compiler only
	}

	file_translate_service_handler(const std::string& file_path)
	: recv_buf_(g_mem_pool_ptr, MAX_RECV_BUF)
	, send_buf_(g_mem_pool_ptr, MAX_RECV_BUF)
	, file_path_(file_path)
	, total_sent_bytes_(0)
	, total_file_bytes_(0)
	{

	}

	virtual i32 open(const cute_socket& socket, cute_reactor* reactor)
	{
		WRITE_INFO_LOG("file_translate_service_handler:open, fd: " + std::to_string(socket.handle()));
		auto ret = cute_service_handler::open(socket, reactor);		
		if (ret == CUTE_ERR)
			return ret;
		ret = this->prepare_file();
                if (ret == CUTE_ERR)
                        return ret;
		ret = this->send_file_size();
                if (ret == CUTE_ERR)
	                return ret;
		this->send_file_content();	
		return CUTE_SUCC;		
	}
	
	virtual i32 handle_input(i32 fd)
	{
		WRITE_INFO_LOG("file_translate_service_handler:handle_input, fd: " + std::to_string(fd));

		for(;;)
		{
			u32 byte_translated = 0;
			auto ret = this->socket_.recv(this->recv_buf_, &byte_translated);
			if (ret == CUTE_ERR) 
			{
				this->close();
				break;
			}
			else if ( byte_translated > 0)
			{
				auto reader_ptr = std::make_shared<reader>();
				
 				// read by data_block iterator
				std::for_each(this->recv_buf_.begin(), this->recv_buf_.end(), [&](cute_data_block& block)
				{
					block.read(reader_ptr);
				});
				

				std::cout << std::endl;
				
				// reset message for reuse
				this->recv_buf_.reset();

				if (ret == CUTE_RECV_BUF_EMPTY) 	// no more data in recv buff
					break;
			}
		}

		return 0;
	}

	virtual i32 handle_output(i32 fd)
	{
		WRITE_INFO_LOG("file_translate_service_handler:handle_output, fd: " + std::to_string(fd));
		this->send_file_content();
		return 0;
	}

	i32 prepare_file()
	{
		this->file_ = std::ifstream(this->file_path_, std::ios::in|std::ios::binary);
		if (this->file_.is_open())
		{
			this->file_.seekg(0, std::ios_base::end);
			this->total_file_bytes_ = this->file_.tellg();
			this->file_.seekg(0, std::ios_base::beg);
			return CUTE_SUCC;
		}
		return CUTE_ERR;
	}

	i32 send_file_size()
	{
		u32 byte_translated = 0;
		this->send_buf_.write(this->total_file_bytes_);
		i32 ret = this->socket_.send(this->send_buf_, &byte_translated);
		this->send_buf_.reset();
		return ret;
	}

	void send_file_content()
	{
		auto file_reader_ptr = std::make_shared<file_reader>(this->file_);
		while (this->total_sent_bytes_ < this->total_file_bytes_)
		{
			if (0 == this->send_buf_.payload_length())	// no data to sent, read data from file
			{
				std::for_each(this->send_buf_.begin(), this->send_buf_.end(), [&](cute_data_block& block)
				{
					block.write(file_reader_ptr);
				});
			}
		
			auto ret = this->send_data();
			if (CUTE_ERR == ret)
			{
				WRITE_INFO_LOG("sent bytes: " + std::to_string(this->total_sent_bytes_) + " file bytes: " + std::to_string(this->total_file_bytes_));
				this->close();
				return;
			}

			if (0 == this->send_buf_.payload_length())	// reset for reuse
				this->send_buf_.reset();
			
			if (CUTE_SEND_BUF_FULL == ret)
				break;
		}	

		if (this->total_sent_bytes_ == this->total_file_bytes_)
			WRITE_INFO_LOG("send operation has been completed succ");
	}

	i32 send_data()
	{
		i32 ret = CUTE_SUCC;
		for (; CUTE_SUCC == ret && this->send_buf_.payload_length() > 0; )	// send succ and has more data to handle
		{				
			u32 byte_translated = 0;
			ret = this->socket_.send(this->send_buf_, &byte_translated);
			if (CUTE_ERR != ret)
				this->total_sent_bytes_ += byte_translated;
		}
		return ret;	
	}		

private:
	cute_message recv_buf_;
	cute_message send_buf_;
	std::string file_path_;
	std::ifstream file_;
	u32 total_sent_bytes_;
	u32 total_file_bytes_;
};


i32 main(i32 argc, char* argv[])
{
	if (argc < 4)
	{
		std::cout << "usage: client <host> <port> <file_path>" << std::endl;
		return 0;
	}

	g_mem_pool_ptr = new cute_mem_pool();
	g_mem_pool_ptr->init(ALLOC_NODE_SIZE, ALLOC_NODE_COUNT);

	cute_reactor reactor;
	reactor.init();
	auto connector = std::make_shared<file_cute_connector>(argv[3]);
	cute_net_addr addr(argv[1], std::atoi(argv[2]));
	connector->connect(addr, &reactor);
	reactor.run_loop();
	return 0;
}




