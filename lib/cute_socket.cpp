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

i32 cute_socket::send(const char * buff, size_t len, size_t* byte_translated)
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

i32 cute_socket::recv(char * buff, size_t len, size_t* byte_translated)
{
	for (ssize_t received = 0; len > 0; buff += received, len -= received)
	{
		received = ::recv(this->fd_, buff, len < MAX_MSS_LEN ? len : MAX_MSS_LEN, 0);
		if (received < 0)			// return error  
			return errno == EAGAIN ? CUTE_RECV_BUF_EMPTY : CUTE_ERR;
		else if (received == 0)		// peer closed
			return CUTE_ERR;
		// received data success
		if (byte_translated)
			*byte_translated += received;
	}

	return CUTE_SUCC;
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

i32 cute_socket::send(cute_message& message, u32* byte_translated)
{
	i32 ret = CUTE_SUCC;
	auto it = message.begin();
	auto it_e = message.end();
	for (; it != it_e; ++it)
	{
		cute_data_block& block = *it;
		ret = this->send(block.raw_data(), block.payload_len(), byte_translated);
		if (CUTE_ERR == ret)
			break;
	};
	return ret;
}

i32 cute_socket::send(cute_message_list& message_list, u32* byte_translated)
{
	i32 ret = CUTE_SUCC;
	auto it = message_list.begin();
	auto it_e = message_list.end();
	for (; it != it_e; ++it)
	{
		cute_message& message = *it;
		if (CUTE_ERR == (ret = this->send(message, byte_translated)))
			break;
	};

	return ret;
}

i32 cute_socket::recv(cute_message_list& message_list, u32* byte_translated)
{
	
	return 0;
}

