add_library (httptth
    io.cpp
    buffers.cpp
    connection.cpp
    text_server.cpp
    http_server.cpp)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

target_include_directories(httptth PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# make pthread a link requirement for this library
target_link_libraries(httptth LINK_PUBLIC pthread)

