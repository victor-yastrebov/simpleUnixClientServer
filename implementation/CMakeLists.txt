project(UDSDB)
cmake_minimum_required(VERSION 2.8)

set(KVD_PROJECT_NAME "kvd")
set(KVCTL_PROJECT_NAME "kvctl")

set(kvd_sources "")
set(kvd_headers "")
set(kvd_directories "")

set(kvctl_sources "")
set(kvctl_headers "")
set(kvctl_directories "")

set(directories ${directories} "${CMAKE_CURRENT_LIST_DIR}")
#message(STATUS "sources: " ${sources})
#message(STATUS "directories: " ${directories})

include_directories(
   ${directories}
)

set(kvd_headers ${kvd_headers}
   "${CMAKE_CURRENT_LIST_DIR}/kvd/Daemon.h"
   "${CMAKE_CURRENT_LIST_DIR}/kvd/UDSServer.h"
   "${CMAKE_CURRENT_LIST_DIR}/UDS.h"
)

set(kvd_sources ${kvd_sources}
   "${CMAKE_CURRENT_LIST_DIR}/kvd/main_kvd.cpp"
   "${CMAKE_CURRENT_LIST_DIR}/kvd/Daemon.cpp"
   "${CMAKE_CURRENT_LIST_DIR}/kvd/UDSServer.cpp"
)

set(kvctl_headers ${kvctl_headers}
   "${CMAKE_CURRENT_LIST_DIR}/UDS.h"
   "${CMAKE_CURRENT_LIST_DIR}/kvctl/UDSClient.h"
)

set(kvctl_sources ${kvctl_sources}
   "${CMAKE_CURRENT_LIST_DIR}/kvctl/main_kvctl.cpp"
   "${CMAKE_CURRENT_LIST_DIR}/kvctl/UDSClient.cpp"
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(${KVD_PROJECT_NAME} ${kvd_headers} ${kvd_sources})
add_executable(${KVCTL_PROJECT_NAME} ${kvctl_headers} ${kvctl_sources})

