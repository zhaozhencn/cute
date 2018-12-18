#ifndef CONFIG_H__
#define CONFIG_H__

#include <stdint.h>

// type define below 
typedef char				i8;
typedef unsigned char			u8;
typedef uint16_t			u16;
typedef uint32_t			u32;
typedef uint64_t			u64;
typedef int16_t				i16;
typedef int32_t				i32;
typedef int64_t				i64;

#define u8_size				sizeof(u8)
#define u16_size			sizeof(u16)
#define u32_size			sizeof(u32)
#define u64_size			sizeof(u64)

#define i8_size				sizeof(i8)
#define i16_size			sizeof(i16)
#define i32_size			sizeof(i32)
#define i64_size			sizeof(i64)



// const define below 
#define CUTE_SUCC			0
#define CUTE_ERR			(-1)
#define CUTE_INVALID_FD			(-1)
#define MAX_EPOLL_EVENTS		64
#define MAX_MSS_LEN			1400
#define MAX_PORT			65535
#define MIN_PORT			100

#define REACTOR_SLEEP_INTERVAL		60		// 60 ms 

#define CUTE_CONNECT_PROCESS		1		// tcp connection is processing
#define CUTE_NO_MORE_CONNECT		2		// have no more establised tcp connection
#define CUTE_RECV_BUF_EMPTY		3		// socket receive buff is empty (have no data to recv)
#define CUTE_SEND_BUF_FULL		4		// socket send buff is full (can't send any more data)	

#define INVALID_TIMER_ID		0

#define ANY_IP				"0.0.0.0"
#define DEFAULT_PORT			0

#define DEFAULT_MAX_BACK_OFF		4

#endif


