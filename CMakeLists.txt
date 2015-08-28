project(HTTPTTH)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS "--std=c++11 -Wall -W")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -O0 -DDEBUG_LOG_LEVEL=7")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(LIBRARY_OUTPUT_PATH    ${CMAKE_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

#
# httptth library
#
add_library (httptth
    src/debug.cpp
    src/io.cpp
    src/buffers.cpp
    src/connection.cpp
    src/text_server.cpp
    src/http_server.cpp)

include_directories("src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

# make pthread a link requirement for this library
target_link_libraries(httptth LINK_PUBLIC pthread)

# and logger too
target_link_libraries (httptth LINK_PUBLIC beamer)

add_subdirectory(beamer)

#
# text server demonstration
#
add_executable(text_server_demo util/text_server_demo.cpp)
target_link_libraries (text_server_demo LINK_PUBLIC httptth)

#
# http server demonstration
#
add_executable(http_server_demo util/http_server_demo.cpp)
target_link_libraries (http_server_demo LINK_PUBLIC httptth)

