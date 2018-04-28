#include "../lib/cute_sche_timer.h"
#include <iostream>

#define TEST_CUTE_TIMER_SCHEDULER
#ifdef TEST_CUTE_TIMER_SCHEDULER

u64 now()
{
	using namespace std::chrono;
	auto now = high_resolution_clock::now();
	return duration_cast<seconds>(now.time_since_epoch()).count();
}

class my_timer_handler : public timer_handler
{
public:
	my_timer_handler(const std::string & content)
		: content_(content)
		, curr_time_(1)
	{

	}

	virtual void exec()
	{
		std::cout << now() << " " << this->content_ << " " << " id: " << this->id_ << " curr time: " << this->curr_time_++ << std::endl;
	}

private:
	std::string content_;
	i32 curr_time_;

};



i32 main()
{
	std::cout << now() << std::endl;

	cute_sche_timer scheduler;
	u64 id1 = scheduler.register_timer(3000, 6, std::make_shared<my_timer_handler>("3000: 3"));
	u64 id2 = scheduler.register_timer(6000, 3, std::make_shared<my_timer_handler>("6000: 3"));
	// scheduler.register_timer(12000, 3, std::make_shared<my_timer_handler>("12000: 3 times"));
	u64 id3 = scheduler.register_timer(8000, 3, std::make_shared<my_timer_handler>("8000: 2"));

	for (auto i=0;; i++)
	{
		auto dispatched_cnt = 0;
		scheduler.dispatch(dispatched_cnt);
		if (dispatched_cnt > 0)
			std::cout << now() << " dispatched_cnt: " << dispatched_cnt << std::endl;
		
		std::chrono::milliseconds duration(60);
		std::this_thread::sleep_for(duration);

		if (i == 200)
		{
			// std::cout << "------------- remove_timer id1: " << id1 << std::endl;
			// scheduler.remove_timer(id1);
			std::cout << "------------- remove_timer id2: " << id2 << std::endl;
			scheduler.remove_timer(id2);
			std::cout << "------------- remove_timer id3: " << id3 << std::endl;
			scheduler.remove_timer(id3);
		}
	}

	return 0;
}



#endif

