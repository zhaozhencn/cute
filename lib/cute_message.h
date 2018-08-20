#ifndef CUTE_MESSAGE_H__
#define CUTE_MESSAGE_H__

#include "config.h"
#include "cute_data_block.h"

class cute_mem_pool;
class cute_message
{
public:
	cute_message(cute_mem_pool* pool, i32 len);
	~cute_message();

	cute_message(cute_message&& src);
	cute_message&& operator = (cute_message&& src);

public:
	cute_message(const cute_message&) = delete;
	cute_message& operator = (const cute_message&) = delete;

public:
	template<typename T>
	bool operator != (T&& compare)
	{
		return static_cast<void*>(this) != static_cast<void*>(&compare);
	}

	template<typename T>
	bool operator == (T&& compare)
	{
		return static_cast<void*>(this) == static_cast<void*>(&compare);
	}

public:
	class data_block_iter
	{
	public:
		data_block_iter(cute_message& message, u32 idx);
		cute_data_block& operator * ();
		data_block_iter& operator ++();

		template<typename T>
		bool operator != (T&& compare)
		{
			return this->message_ != compare.message_ || this->idx_ != compare.idx_;
		}

		template<typename T>
		bool operator == (T&& compare)
		{
			return this->message_ == compare.message_ && this->idx_ == compare.idx_;
		}

	private:
		cute_message & message_;
		u32 idx_;
	};

	data_block_iter begin();
	data_block_iter end();

public:
	inline u32 length() const
	{
		return this->length_;
	}

	bool is_write_full();
	u32 payload_length();
	void reset();

public:
	template<typename T,
		typename = typename std::enable_if<!std::is_pointer<T>::value>>
	i32 read(T&& data)
	{
		return this->read_bytes_i((u8*)&data, sizeof(data));
	}

	template<typename T,
		typename = typename std::enable_if<!std::is_pointer<T>::value>>
	i32 write(T&& data)
	{
	        auto write_vec_idx = this->calc_write_vec_idx();
		return write_vec_idx == this->data_block_vec_.size() ? 0 : this->write_bytes_i((u8*)&data, sizeof(data), write_vec_idx);
	}

	i32 read(u8* data, u32 len);
	i32 write(u8* data, u32 len);
	i32 skip_read(u32 len);

public:
	template<typename T,
		typename = typename std::enable_if<!std::is_pointer<T>::value>>
        i32 peek(T&& data)
        {
                return this->peek_i((u8*)&data, sizeof(data), 0);
        }

private:
	i32 peek_i(u8* data, u32 len, u32 peek_vec_idx);
	i32 write_bytes_i(u8 * data, u32 len, u32 write_vec_idx);
	i32 read_bytes_i(u8* data, u32 len);
	void move_first_block_to_last();
	u32 calc_write_vec_idx();
	i32 skip_read_i(u32 len);

private:
	cute_mem_pool*			pool_;
	std::vector<cute_data_block>	data_block_vec_;
	u32				length_;

	friend class data_block_iter;
};

#endif


