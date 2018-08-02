#include "../lib/cute_mem_pool.h"
#include "../lib/cute_message.h"
#include <iomanip>

int test1()
{
	cute_mem_pool pool;
	pool.init(12, 2);
	cute_message message(&pool, 16);
	for (i32 i = 0; i < 5; ++i)
	{
		auto ret = message.write(i);
		std::cout << "message.write: " << i << " ret: " << ret << std::endl;
	}

	for (i32 i = 0; i < 5; ++i)
	{
		i32 data = 0;
		auto ret = message.peek(data);
		std::cout << "message.peek: " << data << " ret: " << ret << std::endl;
		ret = message.read(data);
		std::cout << "message.read: " << data << " ret: " << ret << std::endl;
	}
	pool.fini();
	
	return 0;
}

int test2()
{
        cute_mem_pool pool;
        pool.init(12, 2);
        cute_message message(&pool, 16);
        for (i32 i = 0; i < 5; ++i)
        {
		u8 ch = '0' + i;
                auto ret = message.write(ch);
                std::cout << "message.write: " << ch << " ret: " << ret << std::endl;
        }

	u32 data = 1234;
	auto ret = message.write(data);
	std::cout << "message.write: " << data << " ret: " << ret << std::endl;

        for (i32 i = 0; i < 5; ++i)
        {
                u8 ch = 0;
		auto ret = message.peek(ch);
		std::cout << "message.peek: " << ch << " ret: " << ret << std::endl;
                ret = message.read(ch);
                std::cout << "message.read: " << ch << " ret: " << ret << std::endl;
        }

	data = 0;
	ret = message.peek(data);
	std::cout << "message.peek: " << data << " ret: " << ret << std::endl;
	ret = message.read(data);
        std::cout << "message.read: " << data << " ret: " << ret << std::endl;

        pool.fini();

        return 0;

}

int test3()
{
	cute_mem_pool pool;
	pool.init(12, 4);
	std::cout << "original pool dump" << std::endl;
	pool.dump();

	{
	std::list<cute_message> mq;
	for (auto k = 0; k < 2; ++k)
	{
	        cute_message message(&pool, 16);
        	for (i32 i = 0; i < 16; ++i)
        	{
			u8 ch = '0' + i;
		        message.write(ch);
		}
		mq.push_back(std::move(message));	
	}

	std::for_each(mq.begin(), mq.end(), [](cute_message& m)
	{
		std::cout << "data in mq: " << std::endl;
		for (i32 i = 0; i < 16; ++i)
	        {
				u8 ch = 0;
				auto ret = m.read(ch);
				std::cout << "message.read: " << ch << " ret: " << ret << std::endl;
	        }
	});
	}

	std::cout << "release pool dump" << std::endl;
	pool.dump();

	return 0;
}




	class reader : public data_block_reader
	{
		virtual u32 execute(u8* read_buf, u32 readable_bytes)
		{
			std::cout << "read_buf: " << std::setbase(16) << static_cast<const void*>(read_buf) << " readable_bytes: " << readable_bytes << std::endl;
			for (auto i = 0; i < readable_bytes; ++i)
				std::cout << read_buf[i] << std::endl;
			return readable_bytes;
		}
	};

	class writer : public data_block_writer
	{
		virtual u32 execute(u8* write_buf, u32 writeable_bytes)
		{
			std::cout << "write_buf: " << std::setbase(16) << static_cast<const void*>(write_buf) << " writeable_bytes: " << writeable_bytes << std::endl;
			for (auto i = 0; i < writeable_bytes; ++i)
			{
				write_buf[i] = '0' + i;
				std::cout << write_buf[i] << std::endl;
			}
			return writeable_bytes;
		}
	};

int test4()
{
	cute_mem_pool pool;
	pool.init(12, 4);
	std::cout << "original pool dump" << std::endl;
	pool.dump();
	cute_message message(&pool, 16);

	auto writer_ptr = std::make_shared<writer>();
	auto reader_ptr = std::make_shared<reader>();

	std::cout << "data_block_writer" << std::endl;
	std::for_each(message.begin(), message.end(), [&](cute_data_block& block)
	{
		block.write(writer_ptr);
	});

	std::cout << "data_block_reader" << std::endl;
	std::for_each(message.begin(), message.end(), [&](cute_data_block& block)
	{
		block.read(reader_ptr);
	});

	pool.fini();
	return 0;
}

// test read and write bytes
int test5()
{
        cute_mem_pool pool;
        pool.init(12, 2);
        cute_message message(&pool, 16); //  message payload byte : 8 (2 int) 

	for (int t = 0; t < 3; ++t)
	{
	        for (i32 i = t; i < t + 5; ++i)
        	{
                	auto ret = message.write((u8*)&i, sizeof(i32));
	                std::cout << "message.write: " << i << " ret: " << ret << std::endl;
        	}

	        for (i32 i = t; i < t + 5; ++i)
        	{
	                i32 data = 0;
                	auto ret = message.peek(data);
        	        std::cout << "--- message.peek: " << data << " ret: " << ret << std::endl;
                	ret = message.read((u8*)&data, sizeof(i32));
	                std::cout << "+++ message.read: " << data << " ret: " << ret << std::endl;
        	}
	}	

        pool.fini();

        return 0;
}

int main()
{
	std::cout << "test1" << std::endl;
	test1();
	std::cout << "test2" << std::endl;
	test2();
	std::cout << "test3" << std::endl;
	test3();
	std::cout << "test4" << std::endl;
	test4();
	std::cout << "test5" << std::endl;
        test5();

	return 0;
}


