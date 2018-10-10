#ifndef CUTE_LOGGER_H__
#define CUTE_LOGGER_H__

#include "inc.h"
#include "config.h"
#include "util.h"


#define LOG_LEVEL_NONE		10
#define LOG_LEVEL_ERROR		3 
#define LOG_LEVEL_DEBUG		2
#define LOG_LEVEL_INFO		1

template<typename LOCK>
class cute_logger
{
public:
	cute_logger(i32 level = LOG_LEVEL_INFO) 
	: level_(level)
	{
	}

public:
	template<typename T>
	void write(T&& log, i32 level)
	{
		std::lock_guard<LOCK> guard(this->mutex_);
		if (level >= this->level_)
			std::cout << std::forward<T>(log) << std::endl;
	}

public:
	std::string get_level_desc(i32 level)
	{
		switch (level)
		{
		case LOG_LEVEL_NONE:
			return "NONE";
		case LOG_LEVEL_INFO:
			return "INFO";
		case LOG_LEVEL_DEBUG: 
			return "DEBUG";
		case LOG_LEVEL_ERROR:
			return "ERROR";
		default:
			return "UNKNOWN";
		}
	}

private:
	i32		level_;
	LOCK		mutex_;
};


template<typename T>
class singleton
{
public:
	static T* inst()
	{
		static T* ins = new T();
		return ins;
	}
};

typedef singleton<cute_logger<std::mutex>> cute_logger_singleton;


#define WRITE_LOG(log, level) \
	{	\
		std::ostringstream os; \
		os << "time: " << util::now() << std::endl; \
		os << "level: " << cute_logger_singleton::inst()->get_level_desc(level) << std::endl;	\
		os << "file: " << __FILE__  <<  " Line: " << __LINE__  << " Function: " << __FUNCTION__ << std::endl;	\
		os << "log: " << log << std::endl; \
		cute_logger_singleton::inst()->write(os.str(), level); \
	}


#define WRITE_INFO_LOG(log)	\
	WRITE_LOG(log, LOG_LEVEL_INFO)

#define WRITE_DEBUG_LOG(log)	\
	WRITE_LOG(log, LOG_LEVEL_DEBUG)

#define WRITE_ERROR_LOG(log)	\
	WRITE_LOG(log, LOG_LEVEL_ERROR)


#endif


