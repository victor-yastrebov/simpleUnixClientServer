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
set(directories ${directories} "${CMAKE_CURRENT_LIST_DIR}/commonLibs/3dparty/asio-1.12.2/include/")

add_definitions(
#    -DUNICODE
#    -D_UNICODE
    -DASIO_STANDALONE
#    -D_WIN32_WINNT=0x0600
    -DASIO_HAS_STD_ARRAY
    -DASIO_HAS_CSTDINT
    -DASIO_HAS_STD_SHARED_PTR
    -DASIO_HAS_STD_ADDRESSOF
    -DASIO_HAS_STD_TYPE_TRAITS
    -DASIO_HAS_STD_ATOMIC
    -DASIO_HAS_LOCAL_SOCKETS
#    -DRH_WINDOW_CLASS_NAME=L"RemoteHelperClient"  # имя класса окна главного окна приложения. Нужно для обнаружения ранее запущенных экземпляров приложения
#    -DBOOST_NO_CXX11_SCOPED_ENUMS                 # for boost::copy_file for CentOS
)

#message(STATUS "sources: " ${sources})
message(STATUS "directories: " ${directories})

include_directories(
   ${directories}
)

set(kvd_headers ${kvd_headers}
#   "${CMAKE_CURRENT_LIST_DIR}/UDS.h"
#   "${CMAKE_CURRENT_LIST_DIR}/kvd/Daemon.h"
   "${CMAKE_CURRENT_LIST_DIR}/kvd/UDSServer.h"
#   "${CMAKE_CURRENT_LIST_DIR}/kvd/DataBase.h"
)

set(kvd_sources ${kvd_sources}
  #"${CMAKE_CURRENT_LIST_DIR}/kvd/main_kvd2.cpp"
  #"${CMAKE_CURRENT_LIST_DIR}/kvd/main_kvd3.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/kvd/main_kvd4.cpp"
  #"${CMAKE_CURRENT_LIST_DIR}/kvd/StreamServer.cpp"
   #"${CMAKE_CURRENT_LIST_DIR}/kvd/main_kvd.cpp"
   #"${CMAKE_CURRENT_LIST_DIR}/kvd/Daemon.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/kvd/UDSServer.cpp"
   #"${CMAKE_CURRENT_LIST_DIR}/kvd/DataBase.cpp"
)

#set(kvctl_headers ${kvctl_headers}
#   "${CMAKE_CURRENT_LIST_DIR}/UDS.h"
#   "${CMAKE_CURRENT_LIST_DIR}/kvctl/UDSClient.h"
#   "${CMAKE_CURRENT_LIST_DIR}/kvd/DataBase.h"
#   "${CMAKE_CURRENT_LIST_DIR}/kvd/Daemon.h"
#)

set(kvctl_sources ${kvctl_sources}
#   "${CMAKE_CURRENT_LIST_DIR}/kvctl/main_kvctl.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/kvctl/main_kvctl2.cpp"
#   "${CMAKE_CURRENT_LIST_DIR}/kvctl/UDSClient.cpp"
#   "${CMAKE_CURRENT_LIST_DIR}/kvd/DataBase.cpp"
#   "${CMAKE_CURRENT_LIST_DIR}/kvd/Daemon.cpp"
)

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -lpthread -static -static-libstdc++ -static-libgcc")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -lpthread -static-libstdc++")

add_executable(${KVD_PROJECT_NAME} ${kvd_headers} ${kvd_sources})
add_executable(${KVCTL_PROJECT_NAME} ${kvctl_headers} ${kvctl_sources})

