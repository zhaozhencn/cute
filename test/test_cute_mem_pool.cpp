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
	auto p1 = pool.acquire();
	std::cout << "after acquire: " << std::setbase(16) << static_cast<const void*>(p1) << " dump: " << std::endl;
	pool.dump();
	auto p2 = pool.acquire();
        std::cout << "after acquire: " << std::setbase(16) << static_cast<const void*>(p2) << " dump: " << std::endl;
        pool.dump();

	std::cout << "release p1: " << static_cast<const void*>(p1) << " dump: " << std::endl;
	pool.release(p1);
	pool.dump();

        std::cout << "release p2: " << static_cast<const void*>(p2) << " dump: " << std::endl;
        pool.release(p2);
        pool.dump();

	pool.fini();
	std::cout << "after fini dump: " << std::endl;
	pool.dump();

	return 0;
}

#endif
