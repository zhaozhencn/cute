cmake_minimum_required (VERSION 2.6)
 
include_directories ("../src")
add_definitions("-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb  -lpthread")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall  -lpthread")
add_executable(client client.cpp)
target_link_libraries (client cute)

include_directories ("../src")
add_definitions("-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb  -lpthread")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall  -lpthread")
add_executable(server server.cpp)
target_link_libraries (server cute)

