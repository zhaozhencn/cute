#ifndef UTIL_H__
#define UTIL_H__

#include "inc.h"

class thread_id_helper
{
public:
	static std::string exec()
	{
		std::ostringstream os;
		os << std::this_thread::get_id();
		return os.str();
	}
};

#endif

