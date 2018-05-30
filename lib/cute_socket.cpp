#include "cute_socket.h"


cute_socket::cute_socket(i32 fd /* = CUTE_INVALID_FD */)
	: fd_(fd)
{
}


cute_socket::~cute_socket()
{
}

i32 cute_socket::open(i32 fd)
{
	this->fd_ = fd;
	return CUTE_ERR;
}

void cute_socket::close()
{
	if (this->fd_ == CUTE_INVALID_FD)
		return;

	::close(this->fd_);
	this->fd_ = CUTE_INVALID_FD;
}

i32 cute_socket::send(u8* buff, u32 len, u32* byte_translated)
{
        for (ssize_t sent = 0; len > 0; buff += sent, len -= sent)
        {
                sent = ::send(this->fd_, buff, len < MAX_MSS_LEN ? len : MAX_MSS_LEN, 0);
                if (sent < 0)
                        return errno == EAGAIN ? CUTE_SEND_BUF_FULL : CUTE_ERR;

                if (byte_translated)
                        *byte_translated += sent;
        }

        return CUTE_SUCC;
}

i32 cute_socket::recv(u8* buff, u32 len, u32* byte_translated)
{
        for (ssize_t received = 0; len > 0; buff += received, len -= received)
        {
                received = ::recv(this->fd_, buff, len < MAX_MSS_LEN ? len : MAX_MSS_LEN, 0);
                if (received < 0) 
                        return errno == EAGAIN ? CUTE_RECV_BUF_EMPTY : CUTE_ERR;
                else if (received == 0)
                        return CUTE_ERR;
                if (byte_translated)
                        *byte_translated += received;
        }

        return CUTE_SUCC;
}

class socket_sender : public data_block_reader
{
public:
	socket_sender(cute_socket& socket)
	: socket_(socket)
	, ret_(CUTE_SUCC)
	{
	}

	u32 execute(u8* read_buf, u32 readable_bytes) override
	{
		u32 byte_translated = 0;
		this->ret_ = this->socket_.send(read_buf, readable_bytes, &byte_translated);		
		return byte_translated;
	}
	
	inline i32 ret() const
	{
		return this->ret_;
	}

private:
	cute_socket& socket_;
	i32 ret_;	
};


i32 cute_socket::send(cute_message& message, u32* byte_translated)
{
	auto sender = std::make_shared<socket_sender>(*this);
	auto it = message.begin();
	auto it_e = message.end();
	for (; it != it_e; ++it)
	{
		auto& block = *it;
		auto ret = block.read(sender);
		if (byte_translated && ret > 0)
			*byte_translated += ret;
		if (sender->ret() != CUTE_SUCC)
			break;
	};
	return sender->ret();
}

class socket_receiver : public data_block_writer
{
public:
	socket_receiver(cute_socket& socket)
        : socket_(socket)
	, ret_(CUTE_SUCC)
        {
        }

	virtual u32 execute(u8* write_buf, u32 writeable_bytes) override
	{
		u32 byte_translated = 0;
                this->ret_ = this->socket_.recv(write_buf, writeable_bytes, &byte_translated);
                return byte_translated;
        }

        inline i32 ret() const
        {
                return this->ret_;
        }
	
private:
        cute_socket& socket_;
        i32 ret_;
};

i32 cute_socket::recv(cute_message& message, u32* byte_translated)
{
	auto receiver = std::make_shared<socket_receiver>(*this);
        auto it = message.begin();
        auto it_e = message.end();
        for (; it != it_e; ++it)
        {
                cute_data_block& block = *it;
		auto ret = block.write(receiver);
                if (byte_translated && ret > 0)
                        *byte_translated += ret;
                if (receiver->ret() != CUTE_SUCC)
                        break;
        };
        return receiver->ret();
}


