#ifndef DATA_BLOCK_H__
#define DATA_BLOCK_H__

#include "config.h"
#include "cute_mem_pool.h"

class cute_data_block
{
public:
	enum { NO_DATA_TO_READ = -1 };
	enum { READ_TO_END = -2 };
	enum { WRITE_TO_END = -3};

public:
	cute_data_block(cute_mem_pool* pool)
		: raw_data_(nullptr)
		, raw_data_len_(0)
		, wp_(0)
		, rp_(0)
		, pool_(pool)
	{

	}

	~cute_data_block()
	{
		// do nothing
	}

public:
	i32 acquire()
	{
		if (nullptr == this->raw_data_)
		{
			this->raw_data_ = this->pool_->acquire();
			this->raw_data_len_ = this->pool_->node_payload_size();
			this->wp_ = this->rp_ = 0;
		}
		return this->raw_data_ ? CUTE_SUCC : CUTE_ERR;
	}

	void release()
	{
		if (this->raw_data_)
		{
			this->pool_->release(this->raw_data_);
			this->raw_data_ = nullptr;
			this->raw_data_len_ = 0;
			this->wp_ = 0;
			this->rp_ = 0;
		}
	}

	// 0 succ, -1 no data to read, -2 reach the buff end, n: part read succ bytes
	i32 read(u8* data, u32 len)
	{
		if (this->rp_ == this->raw_data_len_)
			return READ_TO_END;
		auto readable = this->get_readable_space(len);
		if (0 == readable)
			return NO_DATA_TO_READ;
		memcpy(data, this->raw_data_ + this->rp_, readable);
		this->rp_ += readable;
		return readable == len ? CUTE_SUCC : readable;
	}

	// 0 succ, -1 failed, n: part write succ bytes
	i32 write(u8* data, u32 len)
	{
		auto writeable = this->get_writeable_space(len);
		if (0 == writeable)
			return WRITE_TO_END;
		memcpy(this->raw_data_ + this->wp_, data, writeable);
		this->wp_ += writeable;
		return writeable == len ? CUTE_SUCC : writeable;
	}	

private:
	u32 get_readable_space(u32 len)
	{
		if (this->rp_ >= this->wp_)		// no data to read
			return 0;
		else
			return this->rp_ + len < this->wp_ ? len : this->wp_ - this->rp_;
	}

	u32 get_writeable_space(u32 len)
	{
		return this->wp_ + len < this->raw_data_len_  ? len : this->raw_data_len_ - this->wp_;
	}

private:	
	u8*		raw_data_;
	u32		raw_data_len_;
	u32		wp_;
	u32		rp_;
	cute_mem_pool*	pool_;
};

#endif

