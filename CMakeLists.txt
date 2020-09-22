cmake_minimum_required(VERSION 3.12)
project(LicodeCppClient)

set(CMAKE_CXX_STANDARD 17)

set(THIRD_PARTY_INSTALL_PATH /opt/third_party/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR})
set(THIRD_PARTY_INDEPENDENT_INSTALL FALSE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(cmake/generic.cmake)

include(cmake/external/gflags.cmake)
include(cmake/external/glog.cmake)
include(cmake/external/boost.cmake)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third/json/single_include)

aux_source_directory(messenger MESSENGER_SRCS)
aux_source_directory(thread THREAD_SRCS)

add_executable(LicodeCppClient
        ${MESSENGER_SRCS} ${THREAD_SRCS}
        main.cpp)

target_link_libraries(LicodeCppClient glog gflags pthread)