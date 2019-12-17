project(kvd)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread -std=gnu++0x")
set(SRC_LIST ${CMAKE_CURRENT_LIST_DIR}/kvd/main.cpp)

add_definitions(-DCMAKE_BUILD_TYPE=Debug)
message(STATUS "aaa" ${SRC_LIST})
add_executable(${PROJECT_NAME} ${SRC_LIST})

