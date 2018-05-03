#include "cute_sche_timer.h"
#include "config.h"
#include "cute_logger.h"
#include "util.h"

std::atomic<u64> cute_sche_timer::timer_id_seed_(cute_sche_timer::BASE_TIMER_ID_SEED);

cute_sche_timer::cute_sche_timer()
	: curr_wheel_idx_(0)
	, curr_status_(CLOSED)
{
}

cute_sche_timer::~cute_sche_timer()
{

}

i32 cute_sche_timer::open()
{
	WRITE_INFO_LOG("cute_sche_timer::open, thread: " + thread_id_helper::exec());

	if (this->curr_status_ != cute_sche_timer::CLOSED)
		return CUTE_ERR;

	this->curr_status_ = cute_sche_timer::RUNNING;
	this->thread_ = std::thread([&]()
	{
		for (;;)
		{
			if (cute_sche_timer::ABORTING == this->curr_status_)
				break;

			i32 dispatched_cnt = 0;
			this->dispatch(dispatched_cnt);

			if (dispatched_cnt > 0)
			{
				WRITE_INFO_LOG("cute_sche_timer::dispatch dispatched_cnt: " + std::to_string(dispatched_cnt)
					+ " thread: " + thread_id_helper::exec());
			}

			std::chrono::milliseconds duration(TIMER_INTERVAL);
			std::this_thread::sleep_for(duration);
		}

		WRITE_INFO_LOG("exit cute_sche_timer thread, thread: " + thread_id_helper::exec());
	});
		
	return CUTE_SUCC;
}

void cute_sche_timer::close()
{
	if (cute_sche_timer::RUNNING == this->curr_status_)
	{
		this->curr_status_ = cute_sche_timer::ABORTING;
		this->thread_.join();
		this->thread_ = std::thread();
		this->curr_status_ = cute_sche_timer::CLOSED;
	}
}

u64 cute_sche_timer::register_timer(i32 interval, i32 repeat_cnt, std::shared_ptr<timer_handler> handler)
{
	WRITE_INFO_LOG("cute_sche_timer::register_timer thread: " + thread_id_helper::exec());
	if (interval < TIMER_INTERVAL)
		return INVALID_TIMER_ID;

	u64 timer_id = this->timer_id_seed_++;
	handler->id(timer_id);
	auto node = std::make_shared<timer_node>(timer_id, interval, repeat_cnt, handler, 0);
	return this->register_timer_i(node);
}


u64 cute_sche_timer::register_timer_i(std::shared_ptr<timer_node> node)
{
	WRITE_INFO_LOG("cute_sche_timer::register_timer_i");

	// 1 calc wheel_idx
	auto wheel_idx = node->interval_ / TIMER_INTERVAL;
	node->total_wrap_cnt_ = wheel_idx / MAX_TIMER_WHEEL_SIZE;
	wheel_idx %= MAX_TIMER_WHEEL_SIZE;
	if (0 != node->interval_ % TIMER_INTERVAL)
		++wheel_idx;
	wheel_idx = (this->curr_wheel_idx_ + wheel_idx) % MAX_TIMER_WHEEL_SIZE;

	// 2 save timer_node in timer_wheel_ and mapping in timer_wheel_idx_map_
	try
	{
		std::lock_guard<std::mutex> guard(this->mutex_);

		timer_wheel_t::reference timer_node_map = this->timer_wheel_.at(wheel_idx);
		if (timer_node_map.find(node->id_) != timer_node_map.end()
			|| this->timer_wheel_idx_map_.find(node->id_) != this->timer_wheel_idx_map_.end())
		{
			WRITE_INFO_LOG("exist timer id " + std::to_string(node->id_) + " thread: " + thread_id_helper::exec());
			return INVALID_TIMER_ID;
		}

		timer_node_map.insert(std::make_pair(node->id_, node));
		this->timer_wheel_idx_map_.insert(std::make_pair(node->id_, wheel_idx));
	}
	catch (...)
	{
		return INVALID_TIMER_ID;
	}

	return node->id_;
}

i32 cute_sche_timer::remove_timer(u64 id)
{
	std::lock_guard<std::mutex> guard(this->mutex_);
	WRITE_INFO_LOG("cute_sche_timer::remove_timer: id: " + std::to_string(id));
	this->to_remove_id_list_.push_back(id);
	return CUTE_SUCC;	
}

void cute_sche_timer::dispatch(i32 & dispatched_cnt)
{
	std::list<timer_node_map_t::mapped_type> reschedule_list;
	std::list<u64> to_handler_id_list = this->get_timer_id_list();

	std::for_each(to_handler_id_list.begin(), to_handler_id_list.end(), [&](u64 id)
	{
		try
		{
			timer_node_map_t::mapped_type timer_node = this->get_timer_node(id);
			if (timer_node->curr_wrap_cnt_++ == timer_node->total_wrap_cnt_)		// it is time out now 
			{
				// reset curr_wrap_cnt_;
				timer_node->curr_wrap_cnt_ = 0;

				// handle dispatch
				if (timer_node->repeat_cnt_ > 0)
					--timer_node->repeat_cnt_;

				if (timer_node->handler_)
				{
					++dispatched_cnt;
					i32 ret = timer_node->handler_->exec();	
					if (CUTE_ERR == ret)
					{
						WRITE_ERROR_LOG("timer_node->handler_->exec() ret CUTE_ERR, id: " + std::to_string(id));
						timer_node->status_ = timer_node::INVALID;
					}
				}

				// collect rescheduler node  
				if (timer_node::VALID == timer_node->status_ &&
					(timer_node->repeat_cnt_ > 0 || timer_node->repeat_cnt_ == INFINITE_REPEAT_CNT))
					reschedule_list.push_back(timer_node);

				// remove current node 
				this->rm_timer_node(id);
			}
		}
		catch (u64 id)
		{
			WRITE_ERROR_LOG("timer_node_map NOT exist timer id: " + std::to_string(id));
		}
	});

	// handle rescheduler
	std::for_each(reschedule_list.begin(), reschedule_list.end(), [&](std::shared_ptr<timer_node> timer_node)
	{
		// 当定时器调度周期恰好等于wheel周期的N倍时，本次dispatch完成即认为已经访问第1次（解决： 定时器周期 误增加一个wheel周期的bug )
		auto visit_once = (timer_node->interval_ > 0 && 0 == timer_node->interval_ % (MAX_TIMER_WHEEL_SIZE * TIMER_INTERVAL)) ? 1 : 0;
		timer_node->curr_wrap_cnt_ = visit_once;
		this->register_timer_i(timer_node);
	});

	// handle to remove handler
	this->handle_remove_id_list();

	// update curr_wheel_idx_
	this->curr_wheel_idx_ = (++this->curr_wheel_idx_) % MAX_TIMER_WHEEL_SIZE;
}

std::list<u64> cute_sche_timer::get_timer_id_list()
{
	std::lock_guard<std::mutex> guard(this->mutex_);
	std::list<u64> to_handler_id_list;
	timer_wheel_t::const_reference timer_node_map = this->timer_wheel_[this->curr_wheel_idx_];
	std::for_each(timer_node_map.begin(), timer_node_map.end(), [&](timer_node_map_t::const_reference ref)
	{
		to_handler_id_list.push_back(ref.first);
	});

	return to_handler_id_list;
}

std::shared_ptr<timer_node> cute_sche_timer::get_timer_node(u64 id) throw(u64)
{
	std::lock_guard<std::mutex> guard(this->mutex_);
	timer_wheel_t::const_reference timer_node_map = this->timer_wheel_[this->curr_wheel_idx_];
	if (timer_node_map.find(id) == timer_node_map.end())
		throw id;
	return timer_node_map.at(id);
}

void cute_sche_timer::rm_timer_node(u64 id)
{
	std::lock_guard<std::mutex> guard(this->mutex_);
	this->rm_timer_node_i(id);
}

void cute_sche_timer::rm_timer_node_i(u64 id)
{
	auto it = this->timer_wheel_idx_map_.find(id);
	if (it == this->timer_wheel_idx_map_.end())
		return;

	auto wheel_idx = it->second;
	timer_wheel_t::reference timer_node_map = this->timer_wheel_[wheel_idx];
	timer_node_map.erase(id);
	this->timer_wheel_idx_map_.erase(id);
}

void cute_sche_timer::handle_remove_id_list()
{
	std::lock_guard<std::mutex> guard(this->mutex_);
	std::for_each(this->to_remove_id_list_.begin(), this->to_remove_id_list_.end(), [&](u64 id)
	{
		WRITE_INFO_LOG("cute_sche_timer::rm_timer_node_i id: " + std::to_string(id));
		this->rm_timer_node_i(id);
	});
	this->to_remove_id_list_.clear();
}


