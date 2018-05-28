#include "../lib/cute_sche_timer.h"
#include "../lib/util.h"
#include <iostream>

#define TEST_CUTE_TIMER_SCHEDULER
#ifdef TEST_CUTE_TIMER_SCHEDULER

class my_timer_handler : public timer_handler
{
public:
	my_timer_handler(const std::string & content)
		: content_(content)
		, curr_time_(1)
	{

	}

	virtual i32 exec()
	{
		std::cout << util::now() << " " << this->content_ << " " << " id: " << this->id_ << " curr time: " << this->curr_time_++ << std::endl;
	}

private:
	std::string content_;
	i32 curr_time_;

};



i32 main()
{
	std::cout << util::now() << std::endl;

	cute_sche_timer scheduler;
	scheduler.open();
	// u64 id1 = scheduler.register_timer(3000, std::make_shared<my_timer_handler>("3000: 3"));
	// u64 id2 = scheduler.register_timer(6060, std::make_shared<my_timer_handler>("6060: 3"));
	// scheduler.register_timer(12000, 3, std::make_shared<my_timer_handler>("12000: 3 times"));
	// u64 id3 = scheduler.register_timer(8000, std::make_shared<my_timer_handler>("8000: 2"));
	// u64 id4 = scheduler.register_timer(128000, std::make_shared<my_timer_handler>("128000: 2"));


	u64 id1 = scheduler.register_timer(60, std::make_shared<my_timer_handler>("60"));
	u64 id2 = scheduler.register_timer(240, std::make_shared<my_timer_handler>("240"));
	u64 id3 = scheduler.register_timer(960, std::make_shared<my_timer_handler>("960"));
	u64 id4 = scheduler.register_timer(15360, std::make_shared<my_timer_handler>("15360"));
	u64 id5 = scheduler.register_timer(15360 + 960 + 240 + 60, std::make_shared<my_timer_handler>("15360 + 960 + 240 + 60"));
	u64 id6 = scheduler.register_timer(15360 + 960 + 240 + 60, std::make_shared<my_timer_handler>("15360 + 960 + 240 + 60"));
	// u64 id5 = scheduler.register_timer(15360 * 2, std::make_shared<my_timer_handler>("15360 * 2"));
	// u64 id6 = scheduler.register_timer(15360 * 3, std::make_shared<my_timer_handler>("15360 * 3"));
	// u64 id5 = scheduler.register_timer(3932160, std::make_shared<my_timer_handler>("3932160"));

	std::chrono::milliseconds duration2(100000000);
	std::this_thread::sleep_for(duration2);

	return 0;
}



#endif

