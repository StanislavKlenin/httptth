add_library(logger logger.cpp file.cpp)
#target_include_directories(logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(logger PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                  PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/..")

