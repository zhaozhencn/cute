#ifndef DATA_BLOCK_H__
#define DATA_BLOCK_H__

#include "config.h"
#include "cute_mem_pool.h"

class data_block_reader
{
public:
	virtual ~data_block_reader()
	{
	}

	// return the length of the actual data read
	virtual u32 execute(u8* read_buf, u32 readable_bytes) = 0;
};

using data_block_reader_ptr = std::shared_ptr<data_block_reader>;

class data_block_writer
{
public:
	virtual ~data_block_writer()
	{
	}

	// return the length of the actual data write
	virtual u32 execute(u8* write_buf, u32 writeable_bytes) = 0;
};

using data_block_writer_ptr = std::shared_ptr<data_block_writer>;

class cute_data_block
{
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

	void reset()
	{
		this->wp_ = this->rp_ = 0;
	}

	// Returns the length of the actual data read (0 .. len)
	u32 read(u8* data, u32 len)
	{
		auto readable = this->get_readable_space(len);
		if (readable)
		{
			memcpy(data, this->raw_data_ + this->rp_, readable);
			this->rp_ += readable;
		}
		return readable;
	}

	// Returns the length of the actual data write (0 .. len)
	u32 write(u8* data, u32 len)
	{
		auto writeable = this->get_writeable_space(len);
		if (writeable)
		{
			memcpy(this->raw_data_ + this->wp_, data, writeable);
			this->wp_ += writeable;
		}
		return writeable;
	}

	u32 peek(u8* data, u32 len)
	{
		auto peekable = this->get_readable_space(len);
                if (peekable)
                        memcpy(data, this->raw_data_ + this->rp_, peekable);
                return peekable;
	}

	u32 skip_read(u32 len)
	{
		auto skip = this->get_readable_space(len);
		this->rp_ += skip;
		return skip;
	}

	// read or write by implement interface
	u32 read(data_block_reader_ptr reader)
	{
		auto readable = this->get_readable_space(this->raw_data_len_);
		if (0 == readable)
			return 0;
		auto actual_read = reader->execute(this->raw_data_ + this->rp_, readable);
		if (actual_read >= readable)
			actual_read = readable;
		this->rp_ += actual_read;
		return actual_read;
	}

	u32 write(data_block_writer_ptr writer)
	{
		auto writeable = this->get_writeable_space(this->raw_data_len_);
		if (0 == writeable)
			return 0;	   
		auto actual_write = writer->execute(this->raw_data_ + this->wp_, writeable);
		if (actual_write >= writeable)
			actual_write = writeable;
		this->wp_ += actual_write;
		return actual_write;  
	}

	bool is_write_full()
	{
		return this->wp_ == this->raw_data_len_;
	}

	u32 payload_length()
	{
		return this->wp_ - this->rp_;
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

