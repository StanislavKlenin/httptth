add_executable(server_demo server_demo.cpp)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
target_link_libraries (server_demo LINK_PUBLIC httptth)

