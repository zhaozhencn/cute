#ifndef UTIL_H__
#define UTIL_H__

#include "inc.h"

class thread_id_helper
{
public:
	static std::string exec()
	{
		std::ostringstream os;
		os << std::this_thread::get_id();
		return os.str();
	}
};

class util
{
public:
	static u64 now()
	{
		using namespace std::chrono;
		auto now = high_resolution_clock::now();
		return duration_cast<milliseconds>(now.time_since_epoch()).count();
	}
};

class null_mutex
{
public:
	void lock() 
	{
	} 

	void unlock() noexcept 
	{
	} 

	bool try_lock() 
	{
		return true; 
	} 
};


#endif

