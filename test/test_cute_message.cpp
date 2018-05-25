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
		auto ret = message.read(data);
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
                auto ret = message.read(ch);
                std::cout << "message.read: " << ch << " ret: " << ret << std::endl;
        }

	data = 0;
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

int test4()
{
	cute_mem_pool pool;
	pool.init(12, 4);
	std::cout << "original pool dump" << std::endl;
	pool.dump();
	cute_message message(&pool, 16);

	std::cout << "original pool dump by for" << std::endl;
	auto it = message.begin();
	auto it_e = message.end();
	for (; it != it_e; ++it)
		std::cout << "raw_data: " << std::setbase(16) << static_cast<const void*>((*it).raw_data()) << " raw_data_len: " << (*it).raw_data_len() << std::endl;

	std::cout << "original pool dump by std::for_each" << std::endl;
	std::for_each(message.begin(), message.end(), [](cute_data_block& block)
	{
		std::cout << "raw_data: " << std::setbase(16) << static_cast<const void*>(block.raw_data()) << " raw_data_len: " << block.raw_data_len() << std::endl;
	});

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
	return 0;
}


