#include "cute_mem_pool.h"
#include "cute_data_block.h"
#include "cute_message.h"

cute_message::cute_message(cute_mem_pool* pool, i32 len)
	: pool_(pool)
{
	u32 node_payload_len = pool->node_payload_size();
	u32 node_cnt = len / node_payload_len;
	if (len % node_payload_len)
		++node_cnt;
	for (u32 k = 0; k < node_cnt; ++k)
	{
		auto block = cute_data_block(this->pool_);
		if (CUTE_SUCC == block.acquire())
			this->data_block_vec_.push_back(block);	
	}
}

cute_message::~cute_message()
{
	std::for_each(this->data_block_vec_.begin(), this->data_block_vec_.end(), [](cute_data_block& block)
	{
		block.release();
 	});
	this->data_block_vec_.clear();
}

cute_message::cute_message(cute_message&& src)
: pool_(src.pool_)
, data_block_vec_(std::move(src.data_block_vec_))
{
	src.pool_ = nullptr;
}

cute_message&& cute_message::operator = (cute_message&& src)
{
	this->pool_ = src.pool_;
	this->data_block_vec_ = std::move(src.data_block_vec_);
	src.pool_ = nullptr;
	return std::move(*this);	
}

bool cute_message::is_write_full()
{
	u32 n = this->data_block_vec_.size();
	if (0 == n)
		return true;
	return this->data_block_vec_[n - 1].is_write_full();
}

u32 cute_message::payload_length()
{
	u32 length = 0;
	std::for_each(this->data_block_vec_.begin(), this->data_block_vec_.end(), [&](cute_data_block& block)
	{
		length += block.payload_length();
	});
	return length;
}

void cute_message::reset()
{
	std::for_each(this->data_block_vec_.begin(), this->data_block_vec_.end(), [](cute_data_block& block)
        {
                block.reset();
        });
}

// implement data_block_iter
cute_message::data_block_iter::data_block_iter(cute_message& message, u32 idx)
	: message_(message)
	, idx_(idx)
{

}

cute_data_block& cute_message::data_block_iter::operator * ()
{
	return this->message_.data_block_vec_[this->idx_];
}

cute_message::data_block_iter& cute_message::data_block_iter::operator ++()
{
	++this->idx_;
	return *this;
}
	 
cute_message::data_block_iter cute_message::begin()
{
	return cute_message::data_block_iter(*this, 0);
}

cute_message::data_block_iter cute_message::end()
{
	return cute_message::data_block_iter(*this, this->data_block_vec_.size());
}

// return the bytes read success (0 .. len)
i32 cute_message::read_bytes_i(u8* data, u32 len)
{
	auto&& ref = this->data_block_vec_[0];
	auto acture_read = ref.read((u8*)data, len);
	if (ref.is_write_full() && 0 == ref.payload_length())	// current block has been read completed
		this->move_first_block_to_last();
	if (acture_read > len)		// return error
		return 0;
	else if (0 == acture_read)	// no data to read
		return 0;
	else if (len == acture_read)	// finish read succ
		return len;	
	else  
		return this->read_bytes_i(data + acture_read, len - acture_read) + acture_read;	// read continue remain
}

void cute_message::move_first_block_to_last()
{
	auto it = this->data_block_vec_.begin();
	auto ele = *it;
	this->data_block_vec_.erase(it);
	ele.reset();
	this->data_block_vec_.push_back(ele);
}


// peek_i
i32 cute_message::peek_i(u8* data, u32 len, u32 peek_vec_idx)
{
        if (peek_vec_idx >= this->data_block_vec_.size())
                return 0;
        auto acture_peek = this->data_block_vec_[peek_vec_idx].peek((u8*)data, len);
	if (acture_peek > len)		// return error
		return 0;	
        else if (acture_peek == len)   	// peek succ
                return len;
        else
                return this->peek_i(data + acture_peek, len - acture_peek, peek_vec_idx + 1) + acture_peek; // peek continue remain
}

// return the bytes written success (0 .. len)
i32 cute_message::write_bytes_i(u8* data, u32 len, u32 write_vec_idx)
{
	if (write_vec_idx >= this->data_block_vec_.size())
		return 0;
	auto acture_write = this->data_block_vec_[write_vec_idx].write((u8*)data, len);
	if (acture_write > len)		// return error 
		return 0;	
	else if (acture_write == len)	// write succ
		return len;
	else
		return this->write_bytes_i(data + acture_write, len - acture_write, write_vec_idx + 1) + acture_write; // write continue remain
}

// read and write 
i32 cute_message::read(u8* data, u32 len)
{
	return this->read_bytes_i(data, len);
}

i32 cute_message::write(u8* data, u32 len)
{
	auto write_vec_idx = this->calc_write_vec_idx();
	return (write_vec_idx == this->data_block_vec_.size()) ? 0 : this->write_bytes_i(data, len, write_vec_idx);
}

u32 cute_message::calc_write_vec_idx()
{
        u32 write_vec_idx = 0;
        for(; write_vec_idx < this->data_block_vec_.size(); ++write_vec_idx)
        {
                if (!this->data_block_vec_[write_vec_idx].is_write_full())
                        break;
        }

	return write_vec_idx;
}

// skip bytes for read (0 .. len)
i32 cute_message::skip_read(u32 len)
{
	return this->skip_read_i(len);
}

i32 cute_message::skip_read_i(u32 len)
{
        auto&& ref = this->data_block_vec_[0];
        auto acture_skip = ref.skip_read(len);
        if (ref.is_write_full() && 0 == ref.payload_length())   // current block has been read completed
                this->move_first_block_to_last();
	if (acture_skip > len)		// return error
		return 0;
        else if (0 == acture_skip)	// no data to skip
                return 0;
        else if (len == acture_skip)	// skip succ
                return len;
        else
                return this->skip_read_i(len - acture_skip) + acture_skip; // skip read continue remain

}



