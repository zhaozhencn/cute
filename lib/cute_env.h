#ifndef CUTE_ENV_H__
#define CUTE_ENV_H__

class cute_env
{
public:
	static void init();

public:
	static volatile bool is_shutdown_;
};


#endif
