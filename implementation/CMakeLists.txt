project(kvd)
cmake_minimum_required(VERSION 2.8)

set(sources "")
set(headers "")
set(directories "")


#set(directories ${directories} "${CMAKE_CURRENT_LIST_DIR}/")
#set(directories ${directories} "${CMAKE_CURRENT_LIST_DIR}/kdv/")

message(STATUS "sources: " ${sources})
message(STATUS "directories: " ${directories})

#include_directories(
#   ${directories}/
#)

set(headers ${headers}
   "${CMAKE_CURRENT_LIST_DIR}/kvd/Daemon.h"
   "${CMAKE_CURRENT_LIST_DIR}/kvd/UDSServer.h"
   "${CMAKE_CURRENT_LIST_DIR}/kvd/UDS.h"
)

set(sources ${sources}
   "${CMAKE_CURRENT_LIST_DIR}/kvd/main.cpp"
   "${CMAKE_CURRENT_LIST_DIR}/kvd/Daemon.cpp"
   "${CMAKE_CURRENT_LIST_DIR}/kvd/UDSServer.cpp"
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(${PROJECT_NAME} ${headers} ${sources})

