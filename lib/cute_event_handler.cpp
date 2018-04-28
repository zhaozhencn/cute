#include "cute_event_handler.h"
#include "cute_reactor.h"


cute_event_handler::cute_event_handler()
{

}

cute_event_handler::~cute_event_handler()
{
}

i32 cute_event_handler::handle_input(i32 fd)
{	
	return 0;
}

i32 cute_event_handler::handle_output(i32 fd)
{
	return 0;
}

i32 cute_event_handler::handle_timeout(u64 id)
{
	return 0;
}

i32 cute_event_handler::handle_close(i32 fd)
{
	return 0;
}

