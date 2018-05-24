#ifndef MEM_POOL_H__
#define MEM_POOL_H__

#include "config.h"
#include "inc.h"

class cute_mem_pool
{
public:
	cute_mem_pool()
		: header_(nullptr)
		, tail_(nullptr)
		, alloc_node_size_(0)
		, enlarge_alloc_node_cnt_(0)
	{

	}

	i32 init(u32 alloc_node_size, u32 alloc_node_count)
	{
		if (alloc_node_size <= node::DATA_OFFSET)
			return CUTE_ERR;
		this->alloc_node_size_ = alloc_node_size;
		this->enlarge_alloc_node_cnt_ = alloc_node_count / 2;
		if (0 == this->enlarge_alloc_node_cnt_)
			 this->enlarge_alloc_node_cnt_ = alloc_node_count;
		return this->enlarge_pool(alloc_node_count);
	}

	void fini()
	{
		for (auto p = this->header_; p; )
		{
			auto next = p->next_;
			auto* k = (u8*)p;
			::operator delete[](k);
			p = next;
		}

		this->header_ = this->tail_ = nullptr;
	}

	u8* acquire()
	{
		std::lock_guard<std::mutex> guard(this->mutex_);
		if (nullptr == this->header_ || nullptr == this->tail_)
			return nullptr;
		// if full then enlarge pool
		if (this->header_ == this->tail_ && CUTE_ERR == this->enlarge_pool(this->enlarge_alloc_node_cnt_))
			return nullptr;
		auto p = this->header_;
		this->header_ = this->header_->next_;
		return p->data();
	}

	i32 release(u8* p)
	{
		std::lock_guard<std::mutex> guard(this->mutex_);
		if (nullptr == this->header_ || nullptr == this->tail_)
			return CUTE_ERR;
		p = p - node::DATA_OFFSET;
		u16 magic = *(u16*)p;
		if (node::MAGIC_DATA != magic)
			return CUTE_ERR;
		node* n = (node*)p;
		n->~node();
		this->tail_->next_ = (node*)p;
		this->tail_ = (node*)p;
		return CUTE_SUCC;
	}

	u32 node_payload_size()
	{
		return this->alloc_node_size_ - node::DATA_OFFSET;
	}

        void dump()
        {
                for (auto p = this->header_; p; p = p->next_)
                {
                        std::cout << "p: " << p << " magic: " << p->magic_ << " state: " << p->state_ << " next: " << p->next_ << std::endl;
                }
        }


private:
	i32 enlarge_pool(u32 alloc_node_count)
	{
		node* enlarge_header = nullptr;
		node* enlarge_tail = nullptr;

		bool succ = true;
		for (u32 i = 0; i < alloc_node_count; ++i)
		{
			u8* p = (u8*)::operator new[](this->alloc_node_size_, std::nothrow);
			if (nullptr == p)
			{
				succ = false;
				break;
			}
			node* n = new(p) node();
			if (nullptr == enlarge_header)
				enlarge_header = n;
			if (nullptr != enlarge_tail)
				enlarge_tail->next_ = n;
			enlarge_tail = n;
		}

		if (succ)
		{
			if (nullptr == this->header_)
				this->header_ = enlarge_header;
			if (nullptr == this->tail_)
				this->tail_ = enlarge_tail;
			else
			{
				this->tail_->next_ = enlarge_header;
				this->tail_ = enlarge_header;
			}
			return CUTE_SUCC;
		}

		// release memory already alloc node if failed
		for (auto p = enlarge_header; p; )
		{
			auto next = p->next_;
			auto* k = (u8*)p;
			::operator delete[](k);
			p = next;
		}

		return CUTE_ERR;
	}

private:
	class node
	{
	public:
		enum { MAGIC_DATA = 0xABCD };
		enum { NO_USE = 0, USED = 1 };
		enum { MAGIC_OFFSET = 0 };
		enum { STATE_OFFSET = sizeof(u16) };
		enum { DATA_OFFSET = STATE_OFFSET + sizeof(u16) };

	public:
		node()
			: magic_(MAGIC_DATA)
			, state_(NO_USE)
			, next_(nullptr)
		{

		}

		~node()
		{
			this->magic_ = MAGIC_DATA;
			this->next_ = nullptr;
			this->state_ = NO_USE;
		}

	public:
		u8 * data()
		{
			return (u8*)this + DATA_OFFSET;
		}

	public:
		u16  magic_;     // fixed value: 0xABCD, indicate the node is allocate by this pool
		u16  state_;     // the state of this node, state: NO_USE or USED
		node* next_;      // pointer the next node, the last element of the chain is nullptr
	};

private:
	node*	header_;
	node*	tail_;
	u32	alloc_node_size_;
	u32	enlarge_alloc_node_cnt_;
	std::mutex mutex_;
};


#endif

