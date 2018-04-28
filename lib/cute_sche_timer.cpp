#include "cute_sche_timer.h"
#include "config.h"
#include "cute_logger.h"


u64 cute_sche_timer::timer_id_seed_ = cute_sche_timer::BASE_TIMER_ID_SEED;

cute_sche_timer::cute_sche_timer()
	: curr_wheel_idx_(0)
{
}

cute_sche_timer::~cute_sche_timer()
{
}

u64 cute_sche_timer::register_timer(i32 interval, i32 repeat_cnt, std::shared_ptr<timer_handler> handler)
{
	WRITE_INFO_LOG("cute_sche_timer::register_timer");
	if (interval < MIN_INTERVAL_THRESHOLD)
		return INVALID_TIMER_ID;

	return this->register_timer_i(interval, repeat_cnt, handler, this->timer_id_seed_++);
}


u64 cute_sche_timer::register_timer_i(i32 interval, i32 repeat_cnt, std::shared_ptr<timer_handler> handler, u64 timer_id, i32 curr_wrap_cnt /*= 0*/)
{
	WRITE_INFO_LOG("cute_sche_timer::register_timer_i");	
	handler->id(timer_id);
	timer_node node(timer_id, interval, repeat_cnt, handler, curr_wrap_cnt);
	
	// 1 calc wheel_idx
	auto wheel_idx = interval / TIMER_INTERVAL;
	node.total_wrap_cnt_ = wheel_idx / MAX_TIMER_WHEEL_SIZE;
	wheel_idx %= MAX_TIMER_WHEEL_SIZE;
	if (0 != interval % TIMER_INTERVAL)
		++wheel_idx;
	wheel_idx = (this->curr_wheel_idx_ + wheel_idx) % MAX_TIMER_WHEEL_SIZE;

	// 2 save timer_node in timer_wheel_ and mapping in timer_wheel_idx_map_
	try
	{
		timer_wheel_t::reference timer_node_map = this->timer_wheel_.at(wheel_idx);
		if (timer_node_map.find(timer_id) != timer_node_map.end()
			|| this->timer_wheel_idx_map_.find(timer_id) != this->timer_wheel_idx_map_.end())
			return INVALID_TIMER_ID;

		timer_node_map.insert(std::make_pair(timer_id, node));
		this->timer_wheel_idx_map_.insert(std::make_pair(timer_id, wheel_idx));
	}
	catch (...)
	{
		return INVALID_TIMER_ID;
	}

	return timer_id;
}

i32 cute_sche_timer::remove_timer(u64 id)
{
	WRITE_INFO_LOG("cute_sche_timer::remove_timer: id: " + std::to_string(id));

	auto it = this->timer_wheel_idx_map_.find(id);
	if (it != this->timer_wheel_idx_map_.end())
	{
		auto wheel_idx = it->second;
		timer_wheel_t::reference timer_node_map = this->timer_wheel_[wheel_idx];
		timer_node& timer_node = timer_node_map.at(id);
		timer_node.status_ = timer_node.INVALID;		// set INVALID only
		return CUTE_SUCC;
	}

	return CUTE_ERR;
}

void cute_sche_timer::dispatch(i32 & dispatched_cnt)
{
	std::list<u64> to_delete_id_list;
	std::list<timer_node> reschedule_list;
	timer_wheel_t::reference timer_node_map = this->timer_wheel_[this->curr_wheel_idx_];
	std::for_each(timer_node_map.begin(), timer_node_map.end(), [&](timer_node_map_t::reference ref)
	{
		u64 id = ref.first;
		timer_node& timer_node = ref.second;
		if (timer_node.INVALID == timer_node.status_)  // handle INVALID status first
			to_delete_id_list.push_back(id);
		else if (timer_node.curr_wrap_cnt_++ == timer_node.total_wrap_cnt_)		// it is time out now 
		{
			// reset curr_wrap_cnt_;
			timer_node.curr_wrap_cnt_ = 0;

			// handle dispatch
			if (timer_node.repeat_cnt_ > 0)
				--timer_node.repeat_cnt_;

			if (timer_node.handler_.get() != nullptr)
			{
				timer_node.handler_->exec();
				++dispatched_cnt;
			}

			// collect rescheduler node  
			if (timer_node.repeat_cnt_ > 0 || timer_node.repeat_cnt_ == INFINITE_REPEAT_CNT)
				reschedule_list.push_back(timer_node);

			// collect id for delete
			to_delete_id_list.push_back(id);
		}
	});

	// delete node from to_delete_id_list
	std::for_each(to_delete_id_list.begin(), to_delete_id_list.end(), [&](u64 id)
	{
		timer_node_map.erase(id);
		this->timer_wheel_idx_map_.erase(id);
	});

	// handle rescheduler
	std::for_each(reschedule_list.begin(), reschedule_list.end(), [&](const timer_node& timer_node)
	{
		auto visit_once = (timer_node.interval_ > 0 && 0 == timer_node.interval_ % (MAX_TIMER_WHEEL_SIZE * TIMER_INTERVAL)) ? 1 : 0;
		this->register_timer_i(timer_node.interval_, timer_node.repeat_cnt_, timer_node.handler_, timer_node.id_, visit_once);
		// 当定时器调度周期恰好等于wheel周期的N倍时，当前重新调度完成即认为已经访问第1次（解决： 定时器周期 误增加一个wheel周期的bug )
	});

	// update curr_wheel_idx_
	this->curr_wheel_idx_ = (++this->curr_wheel_idx_) % MAX_TIMER_WHEEL_SIZE;
}



