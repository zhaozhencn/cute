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

class backoff 
{
public:
	backoff(i32 max_idx = DEFAULT_MAX_BACK_OFF)
	: max_idx_(max_idx)
	, curr_idx_(0)
	{		
	}

	i32 get() 
	{
		i32 ret = (1 << this->curr_idx_);
		if (this->curr_idx_ < this->max_idx_)
			 ++this->curr_idx_;
		return ret;
	}

private:
	i32 max_idx_;
	i32 curr_idx_;
};
	
#endif

