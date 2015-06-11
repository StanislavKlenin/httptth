add_executable(server_demo server_demo.cpp)
target_link_libraries (server_demo LINK_PUBLIC httptth)

add_executable(logger_demo logger_demo.cpp)
target_link_libraries (logger_demo LINK_PUBLIC logger)

