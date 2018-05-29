#include "../lib/cute_mem_pool.h"
#include <iomanip>

#define TEST_MEM_POOL
#ifdef TEST_MEM_POOL

int main()
{
	cute_mem_pool pool;
	pool.init(32, 5);
	std::cout << "original dump: " << std::endl;
	pool.dump();

	u8* p[10] = {0};

	for (auto i = 0; i < 5; ++i)
	{
		p[i] = pool.acquire();
		std::cout << "time: " << i << " after acquire: " << std::setbase(16) << static_cast<const void*>(p[i]) << " dump: " << std::endl;
		pool.dump();
	}

	for (auto i = 0; i < 5; ++i)
	{
	        std::cout << "time: " << i << " release p: " << static_cast<const void*>(p[i]) << " dump: " << std::endl;
        	pool.release(p[i]);
	        pool.dump();
	}

	pool.fini();
	std::cout << "after fini dump: " << std::endl;
	pool.dump();

	return 0;
}

#endif
