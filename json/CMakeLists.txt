cmake_minimum_required(VERSION 3.22) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(json STATIC json.cpp)

target_include_directories(json PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})