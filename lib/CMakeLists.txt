cmake_minimum_required (VERSION 2.6)
project (cute)
add_definitions("-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -lpthread")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall  -lpthread")
aux_source_directory(. DIR_LIB_SRCS)
add_library(cute ${DIR_LIB_SRCS})

