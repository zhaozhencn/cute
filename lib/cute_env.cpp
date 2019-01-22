#include "cute_env.h"
#include "cute_logger.h"


static void sig_shutdown_handler(int sig) 
{
	std::string msg;
	switch(sig)
	{
		case SIGINT:
			msg = "received SIGINT to shutdown";
			break;
		case SIGTERM:
			msg = "received SIGTERM to shutdown";
		default:
			msg = "received ABORT SIGINAL to shutdown";
	}

	WRITE_INFO_LOG(msg);	
	cute_env::is_shutdown_ = true;
}


volatile bool cute_env::is_shutdown_ = false;

void cute_env::init()
{
	signal(SIGINT, &sig_shutdown_handler);
        signal(SIGTERM, &sig_shutdown_handler);

        // ignore SIG_PIPE broken signal
        struct sigaction sa;
        sa.sa_handler = SIG_IGN;
        sa.sa_flags = 0;
        sigemptyset(&sa.sa_mask);
	sigaction(SIGPIPE, &sa, 0);
}






