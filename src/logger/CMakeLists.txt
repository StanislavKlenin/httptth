add_library(logger logger.cpp file.cpp)

target_include_directories(logger PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/.."
                                  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
