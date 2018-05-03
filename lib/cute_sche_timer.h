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
	timer_node(u64 id, i32 interval, i32 repeat_cnt, std::shared_ptr<timer_handler> handler, i32 curr_wrap_cnt = 0)
		: id_(id)
		, interval_(interval)
		, repeat_cnt_(repeat_cnt)
		, handler_(handler)
		, curr_wrap_cnt_(curr_wrap_cnt)
		, total_wrap_cnt_(0)
		, status_(VALID)
	{
		;	// do nothing
	}

public:
	u64						id_;
	i32						interval_;
	std::atomic<i32>		repeat_cnt_;
	timer_handler_ptr		handler_;
	std::atomic<i32>		curr_wrap_cnt_;
	i32						total_wrap_cnt_;
	std::atomic<i32>		status_;

public:
	enum { VALID = 1, INVALID = 0 };
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
	u64 register_timer(i32 interval, i32 repeat_cnt, std::shared_ptr<timer_handler> handler);
	i32 remove_timer(u64 id);

public:
	void dispatch(i32 &dispatched_cnt);

private:
	enum { MAX_TIMER_WHEEL_SIZE = 100 };
	enum { TIMER_INTERVAL = 60 };	
	enum { INFINITE_REPEAT_CNT = -1 };
	enum { BASE_TIMER_ID_SEED = 10 };
	enum { CLOSED = 0, RUNNING = 1, ABORTING = 2 };

private:
	typedef std::map<u64, std::shared_ptr<timer_node>>
										timer_node_map_t;
	typedef std::array<timer_node_map_t, MAX_TIMER_WHEEL_SIZE>
										timer_wheel_t;
	typedef std::map<u64, u32>			timer_wheel_idx_map_t;		// pair (timer_id, timer_wheel_id)

private:
	u64 register_timer_i(std::shared_ptr<timer_node> node);
	std::list<u64> get_timer_id_list();
	std::shared_ptr<timer_node> get_timer_node(u64 id) throw(u64);
	void rm_timer_node(u64 id);
	void rm_timer_node_i(u64 id);
	void handle_remove_id_list();

private:	
	timer_wheel_t						timer_wheel_;				
	timer_wheel_idx_map_t				timer_wheel_idx_map_;		// record mapping for timer_id to timer_wheel_id 
	u32									curr_wheel_idx_;
	std::list<u64>						to_remove_id_list_;
	std::mutex							mutex_;

private:
	std::thread							thread_;
	u32									curr_status_;

private:
	static std::atomic<u64>				timer_id_seed_;
};

#endif


