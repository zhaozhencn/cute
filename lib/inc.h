#ifndef INC_H__
#define INC_H__

// stl
#include <list>
#include <vector>
#include <memory>
#include <map>
#include <algorithm>
#include <thread>
#include <iostream>
#include <sstream>
#include <functional>
#include <map>
#include <mutex>
#include <array>
#include <atomic>
#include <set>
#include <new>
#include <queue>

// system
#define LINUX	1
#ifdef LINUX
#include <unistd.h>  
#include <errno.h>  
#include <sys/socket.h>  
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>  
#include <netdb.h>
#include <sys/epoll.h>  
#include <string.h>
#include <signal.h>
#endif

#endif

