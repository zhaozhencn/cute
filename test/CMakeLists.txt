cmake_minimum_required (VERSION 2.6)
 
include_directories ("../src")
add_definitions("-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb  -lpthread")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -g -lpthread")
add_executable(test_cute_acceptor test_cute_acceptor.cpp)
target_link_libraries (test_cute_acceptor cute)

include_directories ("../src")
add_definitions("-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb  -lpthread")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -g -lpthread")
add_executable(test_cute_connector test_cute_connector.cpp)
target_link_libraries (test_cute_connector cute)

include_directories ("../src")
add_definitions("-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb  -lpthread")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -g -lpthread")
add_executable(test_cute_sche_timer test_cute_sche_timer.cpp)
target_link_libraries (test_cute_sche_timer cute)

include_directories ("../src")
add_definitions("-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb  -lpthread")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -g -lpthread")
add_executable(test_cute_mem_pool test_cute_mem_pool.cpp)
target_link_libraries (test_cute_mem_pool cute)

include_directories ("../src")
add_definitions("-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb  -lpthread")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -g -lpthread")
add_executable(test_cute_message test_cute_message.cpp)
target_link_libraries (test_cute_message cute)

include_directories ("../src")
add_definitions("-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb  -lpthread")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -g -lpthread")
add_executable(test_cute_async_task test_cute_async_task.cpp)
target_link_libraries (test_cute_async_task cute)
