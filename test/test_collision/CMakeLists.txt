cmake_minimum_required(VERSION 3.22) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(test_collision)

file(GLOB GEN_SOURCES ../gen/*/*.cpp)

add_executable(test_collision test_collision.cpp ${GEN_SOURCES})
target_include_directories(test_collision PRIVATE ..)
target_link_libraries(test_collision PRIVATE engine)

install(TARGETS test_collision DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/deploy)