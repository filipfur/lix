cmake_minimum_required(VERSION 3.22) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(test_basic)

file(GLOB GEN_SOURCES ../gen/*/*.cpp)

add_executable(test_basic test.cpp ${GEN_SOURCES})
target_include_directories(test_basic PRIVATE ..)
target_link_libraries(test_basic PRIVATE engine)

install(TARGETS test_basic DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/deploy)