#ifndef CUTE_SCHE_TIMER_H__
#define CUTE_SCHE_TIMER_H__

#include "inc.h"
#include "config.h"


class timer_handler : public std::enable_shared_from_this<timer_handler>
{
public:
	virtual ~timer_handler()
	{
		;
	}

public:
	u64 id() const
	{
		return id_;
	}

	void id(u64 id)
	{
		this->id_ = id;
	}

public:
	virtual i32 exec() = 0;

protected:
	u64	id_;
};

class timer_node
{
public:
	typedef std::shared_ptr<timer_handler> timer_handler_ptr;

public:
	timer_node(u64 id, i32 interval, std::shared_ptr<timer_handler> handler)
		: id_(id)
		, full_interval_(interval)
		, lower_magnitude_interval_(interval)
		, handler_(handler)
		, status_(VALID)
	{
		;	// do nothing
	}

public:
	u64						id_;
	i32						full_interval_;
	i32						lower_magnitude_interval_;
	timer_handler_ptr		handler_;
	i32						status_;

public:
	enum { VALID = 1, INVALID = 0 };
};

class cute_sche_timer;
class timer_wheel
{
public:
	typedef std::shared_ptr<timer_node>		timer_node_ptr_t;
	typedef std::map<u64, timer_node_ptr_t> timer_node_map_t;
	typedef std::vector<timer_node_map_t>	timer_node_map_vec_t;
	typedef std::map<u64, u32>				timer_wheel_idx_map_t;		// pair (timer_id, timer_wheel_idx)

public:
	timer_wheel(u32 max_wheel_size, u32 magnitude, std::weak_ptr<timer_wheel> lower_magnitude_wheel_ptr, cute_sche_timer* sche_timer);

public:
	inline u32 curr_wheel_idx() const
	{
		return this->curr_wheel_idx_;
	}

	inline u32 max_wheel_size() const
	{
		return this->max_wheel_size_;
	}

public:
	void move_curr_wheel_idx();
	i32 add_timer_node(timer_node_ptr_t node, u32 wheel_idx);
	void remove_timer_node(u64 id);
	void down_magnitude_entry();	
	std::list<u64> get_timer_id_list();
	std::shared_ptr<timer_node> get_timer_node(u64 id);

private:
	i32 add_timer_node_i(timer_node_ptr_t node, u32 wheel_idx);
	void remove_timer_node_i(u64 id);
	void down_magnitude_timer_node_i(timer_node_ptr_t node);

private:
	u32							max_wheel_size_;
	u32							magnitude_;
	u32							curr_wheel_idx_;

private:
	timer_node_map_vec_t		vec_;
	timer_wheel_idx_map_t		timer_wheel_idx_map_;
	std::weak_ptr<timer_wheel>	lower_magnitude_wheel_ptr_;
	cute_sche_timer*			sche_timer_;

private:
	std::mutex					mutex_;
};


class cute_sche_timer
{
public:
	cute_sche_timer();
	~cute_sche_timer();

public:
	i32 open();
	void close();

public:
	u64 register_timer(i32 interval, std::shared_ptr<timer_handler> handler);
	i32 remove_timer(u64 id);

public:
	u32 get_magnitude_from_map(u64 id);
	void register_timer_id_to_map(u64 id, u32 magnitude);
	void remove_timer_id_from_map(u64 id);
	
public:
	enum { TIMER_INTERVAL = 60 };
	enum { MAX_TIMER_INVTERVAL = 1 << 30 };
	enum { BASE_TIMER_ID_SEED = 10 };
	enum { CLOSED = 0, RUNNING = 1, ABORTING = 2 };
	enum {
		MAGNITUDE_1 = 1,
		MAGNITUDE_8 = (1 << 8),
		MAGNITUDE_12 = (1 << 12),
		MAGNITUDE_16 = (1 << 16),
		MAGNITUDE_24 = (1 << 24)
	};
	enum {
		MAGNITUDE_1_MAX_SIZE = (1 << 8),
		MAGNITUDE_8_MAX_SIZE = (1 << 4),
		MAGNITUDE_12_MAX_SIZE = (1 << 4),
		MAGNITUDE_16_MAX_SIZE = (1 << 8),
		MAGNITUDE_24_MAX_SIZE = (1 << 8)
	};

private:
	void dispatch(i32 &dispatched_cnt);
	void update();
	u64 add_timer_node(std::shared_ptr<timer_node> node);
	void move_magnitude_wheel_idx();
	void try_down_timer_magnitude();
	std::list<u64> get_timer_id_list();
	std::shared_ptr<timer_node> get_timer_node(u64 id) throw(u64);
	

private:
	typedef std::map<u32, std::shared_ptr<timer_wheel>>
		timer_wheel_map_t;
	timer_wheel_map_t					timer_wheel_map_;
	u32									curr_tick_;

	typedef std::map<u64, u32>			timer_idx_wheel_map_t;		// map timer_id to timer_wheel
	timer_idx_wheel_map_t				timer_idx_wheel_map_;
	std::mutex							idx_wheel_map_mutex_;

	friend class timer_wheel;

private:
	std::thread							thread_;
	u32									curr_status_;

private:
	static std::atomic<u64>				timer_id_seed_;
};

#endif


