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

#set(directories ${directories} ${CMAKE_CURRENT_LIST_DIR})
set(directories ${directories} "${CMAKE_CURRENT_LIST_DIR}/commonLibs/")
set(directories ${directories} "${CMAKE_CURRENT_LIST_DIR}/commonLibs/3dparty/asio-1.12.2/include/")

add_definitions(
    -DASIO_STANDALONE
    -DASIO_HAS_STD_ARRAY
    -DASIO_HAS_CSTDINT
    -DASIO_HAS_STD_SHARED_PTR
    -DASIO_HAS_STD_ADDRESSOF
    -DASIO_HAS_STD_TYPE_TRAITS
    -DASIO_HAS_STD_ATOMIC
    -DASIO_HAS_LOCAL_SOCKETS
)

message(STATUS "directories: " ${directories})

include_directories(
   ${directories}
)

set(kvd_headers ${kvd_headers}
   "${CMAKE_CURRENT_LIST_DIR}/kvd/Daemon.h"
   "${CMAKE_CURRENT_LIST_DIR}/kvd/SysLogger.h"
   "${CMAKE_CURRENT_LIST_DIR}/kvd/UDSServer.h"
   "${CMAKE_CURRENT_LIST_DIR}/kvd/Session.h"
   "${CMAKE_CURRENT_LIST_DIR}/kvd/DataBase.h"
   "${CMAKE_CURRENT_LIST_DIR}/commonLibs/AppProtocol.h"
   "${CMAKE_CURRENT_LIST_DIR}/commonLibs/UDS.h"
)

set(kvd_sources ${kvd_sources}
  "${CMAKE_CURRENT_LIST_DIR}/kvd/main_kvd.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/kvd/Daemon.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/kvd/UDSServer.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/kvd/DataBase.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/kvd/Session.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/commonLibs/AppProtocol.cpp"
)

set(kvctl_headers ${kvctl_headers}
#   "${CMAKE_CURRENT_LIST_DIR}/kvctl/UDSClient.h"
   "${CMAKE_CURRENT_LIST_DIR}/commonLibs/UDS.h"
   "${CMAKE_CURRENT_LIST_DIR}/commonLibs/AppProtocol.h"
)

set(kvctl_sources ${kvctl_sources}
    "${CMAKE_CURRENT_LIST_DIR}/kvctl/main_kvctl.cpp"
#   "${CMAKE_CURRENT_LIST_DIR}/kvctl/UDSClient.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/commonLibs/AppProtocol.cpp"
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -lpthread -static-libstdc++")

add_executable(${KVD_PROJECT_NAME} ${kvd_headers} ${kvd_sources})
add_executable(${KVCTL_PROJECT_NAME} ${kvctl_headers} ${kvctl_sources})