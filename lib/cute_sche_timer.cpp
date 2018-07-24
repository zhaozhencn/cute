#include "cute_sche_timer.h"
#include "config.h"
#include "cute_logger.h"
#include "util.h"

// implement timer_wheel 
timer_wheel::timer_wheel(u32 max_wheel_size, u32 magnitude, std::weak_ptr<timer_wheel> lower_magnitude_wheel_ptr,
	cute_sche_timer* sche_timer)
	: max_wheel_size_(max_wheel_size)
	, magnitude_(magnitude)
	, curr_wheel_idx_(0)
	, lower_magnitude_wheel_ptr_(lower_magnitude_wheel_ptr)
	, sche_timer_(sche_timer)
{
	vec_.reserve(max_wheel_size);
	for (u32 i = 0; i < this->max_wheel_size_; ++i)
		vec_.push_back(timer_node_map_t());
}

void timer_wheel::move_curr_wheel_idx()
{
	std::lock_guard<std::mutex> guard(this->mutex_);
	this->curr_wheel_idx_ = (++this->curr_wheel_idx_) % this->max_wheel_size_;
}

i32 timer_wheel::add_timer_node(timer_node_ptr_t node, u32 wheel_idx)
{
	std::lock_guard<std::mutex> guard(this->mutex_);
	return this->add_timer_node_i(node, wheel_idx);
}

i32 timer_wheel::add_timer_node_i(timer_node_ptr_t node, u32 wheel_idx)
{
	if (this->timer_wheel_idx_map_.find(node->id_) != this->timer_wheel_idx_map_.end())
		return CUTE_ERR;

	wheel_idx = (this->curr_wheel_idx() + wheel_idx) % this->max_wheel_size();
	timer_node_map_vec_t::reference map = this->vec_.at(wheel_idx);
	map.insert(std::make_pair(node->id_, node));
	this->timer_wheel_idx_map_.insert(std::make_pair(node->id_, wheel_idx));

	// update timer id to timer_wheel
	this->sche_timer_->register_timer_id_to_map(node->id_, this->magnitude_);
	return CUTE_SUCC;
}

void timer_wheel::remove_timer_node(u64 id)
{
	std::lock_guard<std::mutex> guard(this->mutex_);
	this->remove_timer_node_i(id);
}

void timer_wheel::remove_timer_node_i(u64 id)
{
	auto it = this->timer_wheel_idx_map_.find(id);
	if (it == this->timer_wheel_idx_map_.end())
		return;

	auto wheel_idx = it->second;
	timer_node_map_vec_t::reference map = this->vec_.at(wheel_idx);
	map.erase(id);
	this->timer_wheel_idx_map_.erase(id);
}

void timer_wheel::down_magnitude_entry()
{
	std::lock_guard<std::mutex> guard(this->mutex_);

	auto ptr = this->lower_magnitude_wheel_ptr_.lock();
	timer_node_map_vec_t::reference map = this->vec_.at(this->curr_wheel_idx_);
	std::for_each(map.begin(), map.end(), [&](timer_node_map_t::const_reference ref)
	{
		auto node = ref.second;
		if (ptr)
			ptr->down_magnitude_timer_node_i(node);
		this->timer_wheel_idx_map_.erase(node->id_);
	});

	map.clear();	
}

void timer_wheel::down_magnitude_timer_node_i(timer_node_ptr_t node)
{
	auto wheel_idx = node->lower_magnitude_interval_ / cute_sche_timer::TIMER_INTERVAL / this->magnitude_;

	// current magnitude can hold it
	if (wheel_idx > 0)	
	{
		node->lower_magnitude_interval_ -= wheel_idx * this->magnitude_ * cute_sche_timer::TIMER_INTERVAL;
		this->add_timer_node_i(node, wheel_idx);
	}
	else
	{
		WRITE_INFO_LOG("down_magnitude_timer_node_i");
		auto ptr = this->lower_magnitude_wheel_ptr_.lock();
		if (ptr)
			ptr->down_magnitude_timer_node_i(node);
		else
		{
			WRITE_INFO_LOG("reach the wheel chain head");
			this->add_timer_node_i(node, wheel_idx); // special: reach the wheel chain head
		}
	}
}

std::list<u64> timer_wheel::get_timer_id_list()
{
	std::lock_guard<std::mutex> guard(this->mutex_);
	std::list<u64> to_handler_id_list;
	timer_node_map_vec_t::const_reference timer_node_map = this->vec_.at(this->curr_wheel_idx_);
	std::for_each(timer_node_map.begin(), timer_node_map.end(), [&to_handler_id_list](timer_node_map_t::const_reference ref)
	{
		to_handler_id_list.push_back(ref.first);
	});

	return to_handler_id_list;
}

std::shared_ptr<timer_node> timer_wheel::get_timer_node(u64 id)
{
	std::lock_guard<std::mutex> guard(this->mutex_);
	timer_node_map_vec_t::reference timer_node_map = this->vec_.at(this->curr_wheel_idx_);
	if (timer_node_map.find(id) == timer_node_map.end())
		throw id;
	return timer_node_map.at(id);
}


// implement cute_sche_timer
std::atomic<u64> cute_sche_timer::timer_id_seed_(cute_sche_timer::BASE_TIMER_ID_SEED);

cute_sche_timer::cute_sche_timer()
	: curr_tick_(0)
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

	std::vector<std::pair<u32, u32>> magnitude_vec = 
	{
		{ MAGNITUDE_1, MAGNITUDE_1_MAX_SIZE},
		{ MAGNITUDE_8, MAGNITUDE_8_MAX_SIZE },
		{ MAGNITUDE_12, MAGNITUDE_12_MAX_SIZE },
		{ MAGNITUDE_16, MAGNITUDE_16_MAX_SIZE },
		{ MAGNITUDE_24, MAGNITUDE_24_MAX_SIZE }
	};

	std::shared_ptr<timer_wheel> prev_timer_wheel;
	std::for_each(magnitude_vec.begin(), magnitude_vec.end(), [&](const std::pair<u32, u32>& value)
	{
		auto max_wheel_size = value.second;
		auto magnitude = value.first;
		auto weak_ptr = std::weak_ptr<timer_wheel>(prev_timer_wheel);
		auto wheel = std::make_shared<timer_wheel>(max_wheel_size, magnitude, weak_ptr, this);
		this->timer_wheel_map_.insert(std::make_pair(magnitude, wheel));
		prev_timer_wheel = wheel;
	});


	this->curr_status_ = cute_sche_timer::RUNNING;
	this->thread_ = std::thread([&]()
	{
		for (;;)
		{
			if (cute_sche_timer::ABORTING == this->curr_status_)
				break;

			// dispatch timer 
			i32 dispatched_cnt = 0;
			this->dispatch(dispatched_cnt);

			if (dispatched_cnt > 0)
			{
				WRITE_INFO_LOG("cute_sche_timer::dispatch dispatched_cnt: " + std::to_string(dispatched_cnt)
					+ " thread: " + thread_id_helper::exec());
			}

			// sleep timer interval 
			std::chrono::milliseconds duration(TIMER_INTERVAL);
			std::this_thread::sleep_for(duration);

			// update 
			this->update();
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

u64 cute_sche_timer::register_timer(i32 interval, std::shared_ptr<timer_handler> handler)
{	
	WRITE_INFO_LOG("cute_sche_timer::register_timer thread: " + thread_id_helper::exec());
	if (interval < TIMER_INTERVAL)
		interval = TIMER_INTERVAL;
	else if (interval > MAX_TIMER_INVTERVAL)
		return INVALID_TIMER_ID;

	u64 timer_id = this->timer_id_seed_++;
	handler->id(timer_id);
	auto node = std::make_shared<timer_node>(timer_id, interval, handler);
	return this->add_timer_node(node);
}


u64 cute_sche_timer::add_timer_node(std::shared_ptr<timer_node> node)
{
	WRITE_INFO_LOG("cute_sche_timer::add_timer_node thread: " + thread_id_helper::exec());
	std::vector<u32> magnitude_vec = { MAGNITUDE_24, MAGNITUDE_16, MAGNITUDE_12, MAGNITUDE_8, MAGNITUDE_1 };
	for (auto it = magnitude_vec.begin(); it != magnitude_vec.end(); ++it)
	{
		auto magnitude = *it;
		auto wheel_idx = node->lower_magnitude_interval_ / TIMER_INTERVAL / magnitude;
		if (wheel_idx > 0)
		{
			node->lower_magnitude_interval_ -= wheel_idx * magnitude * TIMER_INTERVAL;
			auto timer_wheel = this->timer_wheel_map_.at(magnitude);
			timer_wheel->add_timer_node(node, wheel_idx);
			return node->id_;
		}
	};

	return INVALID_TIMER_ID;
}

i32 cute_sche_timer::remove_timer(u64 id)
{
	WRITE_INFO_LOG("cute_sche_timer::remove_timer: id: " + std::to_string(id));
	
	try
	{
		auto magnitude = this->get_magnitude_from_map(id);
		auto wheel = this->timer_wheel_map_.at(magnitude);
		WRITE_INFO_LOG("cute_sche_timer::remove_timer id: " + std::to_string(id));
		wheel->remove_timer_node(id);
		this->remove_timer_id_from_map(id);
	}
	catch (...)
	{
		WRITE_INFO_LOG("cute_sche_timer::remove_timer NOT exist id: " + std::to_string(id));
	}

	return CUTE_SUCC;	
}


u32 cute_sche_timer::get_magnitude_from_map(u64 id)
{
	std::lock_guard<std::mutex> guard(this->idx_wheel_map_mutex_);
	return this->timer_idx_wheel_map_.at(id);
}

void cute_sche_timer::register_timer_id_to_map(u64 id, u32 magnitude)
{
	std::lock_guard<std::mutex> guard(this->idx_wheel_map_mutex_);
	this->timer_idx_wheel_map_[id] = magnitude;
}

void cute_sche_timer::remove_timer_id_from_map(u64 id)
{
	std::lock_guard<std::mutex> guard(this->idx_wheel_map_mutex_);
	this->timer_idx_wheel_map_.erase(id);
}

void cute_sche_timer::dispatch(i32 & dispatched_cnt)
{
	// try down magnitude from large to small 
	this->try_down_timer_magnitude();

	// handle dispatch
	std::list<u64> to_handler_id_list = this->get_timer_id_list();
	std::for_each(to_handler_id_list.begin(), to_handler_id_list.end(), [&](u64 id)
	{
		try
		{
			auto timer_node = this->get_timer_node(id);			
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

			this->remove_timer(id);
		}
		catch (u64 id)
		{
			WRITE_ERROR_LOG("timer_node_map NOT exist timer id: " + std::to_string(id));
		}
	});	
}

void cute_sche_timer::update()
{
	// update move_magnitude_wheel_idx
	this->move_magnitude_wheel_idx();

	// update curr_tick_
	this->curr_tick_ = (++this->curr_tick_) % MAX_TIMER_INVTERVAL;
}

void cute_sche_timer::move_magnitude_wheel_idx()
{
	std::for_each(this->timer_wheel_map_.rbegin(), this->timer_wheel_map_.rend(), [&](timer_wheel_map_t::reference ref)
	{
		u32 magnitude = ref.first;
		auto timer_wheel = ref.second;
		if (0 == this->curr_tick_ % magnitude)
			timer_wheel->move_curr_wheel_idx();
	});
}

void cute_sche_timer::try_down_timer_magnitude()
{
	std::for_each(this->timer_wheel_map_.rbegin(), this->timer_wheel_map_.rend(), [&](timer_wheel_map_t::reference ref)
	{
		u32 magnitude = ref.first;
		auto timer_wheel = ref.second;
		if (MAGNITUDE_1 != magnitude && 0 == this->curr_tick_ % magnitude)
			timer_wheel->down_magnitude_entry();
	});
}

std::list<u64> cute_sche_timer::get_timer_id_list()
{
	auto wheel = this->timer_wheel_map_.at(cute_sche_timer::MAGNITUDE_1);
	return wheel->get_timer_id_list();
}

std::shared_ptr<timer_node> cute_sche_timer::get_timer_node(u64 id) throw(u64)
{
	auto wheel = this->timer_wheel_map_.at(cute_sche_timer::MAGNITUDE_1);
	return wheel->get_timer_node(id);
}

