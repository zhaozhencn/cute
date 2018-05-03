#ifndef CUTE_LOGGER_H__
#define CUTE_LOGGER_H__

#include "inc.h"
#include "config.h"

class cute_logger
{
public:
	enum { NONE = 10, ERROR = 3, DEBUG = 2, INFO = 1};
	cute_logger(i32 level = INFO);
	~cute_logger();

public:
	template<typename T>
	void write(T&& log, i32 level)
	{
		std::lock_guard<std::mutex> guard(this->mutex_);
		if (level >= this->level_)
			std::cout << std::forward<T>(log) << std::endl;
	}

public:
	std::string get_level_desc(i32 level)
	{
		switch (level)
		{
		case NONE:
			return "NONE";
		case INFO:
			return "INFO";
		case DEBUG: 
			return "DEBUG";
		case ERROR:
			return "ERROR";
		default:
			return "UNKNOWN";
		}
	}

private:
	i32			level_;
	std::mutex	mutex_;
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

typedef singleton<cute_logger> cute_logger_singleton;


#define WRITE_LOG(log, level) \
	{	\
		std::ostringstream os; \
		os << "Level: " << cute_logger_singleton::inst()->get_level_desc(level) << std::endl;	\
		os << "File: " << __FILE__  <<  " Line: " << __LINE__  << " Function: " << __FUNCTION__ << std::endl;	\
		os << "Log: " << log << std::endl; \
		cute_logger_singleton::inst()->write(os.str(), level); \
	}


#define WRITE_INFO_LOG(log)	\
	WRITE_LOG(log, cute_logger::INFO)

#define WRITE_DEBUG_LOG(log)	\
	WRITE_LOG(log, cute_logger::DEBUG)

#define WRITE_ERROR_LOG(log)	\
	WRITE_LOG(log, cute_logger::ERROR)


#endif


