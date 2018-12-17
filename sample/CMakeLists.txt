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

include_directories ("../src")
add_definitions("-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb  -lpthread")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall  -lpthread")
add_executable(file_translate_client file_translate_client.cpp)
target_link_libraries (file_translate_client cute)

include_directories ("../src")
add_definitions("-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb  -lpthread")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall  -lpthread")
add_executable(file_translate_server file_translate_server.cpp)
target_link_libraries (file_translate_server cute)

include_directories ("../src")
add_definitions("-Wall -std=c++11 -I/usr/local/include")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb  -lpthread -L/usr/local/lib -lprotobuf")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall  -g -lpthread -L/usr/local/lib -lprotobuf")
add_executable(pb_client pb_client.cpp address.pb.cpp)
target_link_libraries (pb_client cute protobuf)

include_directories ("../src")
add_definitions("-Wall -std=c++11 -I/usr/local/include")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb  -lpthread -L/usr/local/lib -lprotobuf")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall  -g -lpthread -L/usr/local/lib -lprotobuf")
add_executable(pb_server pb_server.cpp address.pb.cpp)
target_link_libraries (pb_server cute protobuf)



