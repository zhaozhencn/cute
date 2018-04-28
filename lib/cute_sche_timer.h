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
	virtual void exec() = 0;

protected:
	u64	id_;
};


class cute_sche_timer
{
public:
	cute_sche_timer();
	~cute_sche_timer();

public:
	u64 register_timer(i32 interval, i32 repeat_cnt, std::shared_ptr<timer_handler> handler);
	i32 remove_timer(u64 id);

public:
	void dispatch(i32 &dispatched_cnt);

private:
	u64 register_timer_i(i32 interval, i32 repeat_cnt, std::shared_ptr<timer_handler> handler, u64 timer_id, i32 curr_wrap_cnt = 0);

private:
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
		i32						repeat_cnt_;
		timer_handler_ptr		handler_;
		i32						curr_wrap_cnt_;
		i32						total_wrap_cnt_;	
		i32						status_;			

	public:
		enum { VALID = 1, INVALID = 0};
	};


private:
	enum { MAX_TIMER_WHEEL_SIZE = 100 };
	enum { TIMER_INTERVAL = 60 };	
	enum { INFINITE_REPEAT_CNT = -1 };
	enum { MIN_INTERVAL_THRESHOLD = 60 };
	enum { BASE_TIMER_ID_SEED = 10 };

private:
	typedef std::map<u64, timer_node>	timer_node_map_t;
	typedef std::array<timer_node_map_t, MAX_TIMER_WHEEL_SIZE> 
										timer_wheel_t;
	typedef std::map<u64, u32>			timer_wheel_idx_map_t;		// pair (timer_id, timer_wheel_id)

private:	
	timer_wheel_t						timer_wheel_;				
	timer_wheel_idx_map_t				timer_wheel_idx_map_;		// record mapping for timer_id to timer_wheel_id 
	u32									curr_wheel_idx_;

private:
	static u64							timer_id_seed_;
};

#endif


