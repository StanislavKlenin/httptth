add_executable(demo test.cpp)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
target_link_libraries (demo LINK_PUBLIC httptth)

