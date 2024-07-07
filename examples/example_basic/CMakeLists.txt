cmake_minimum_required(VERSION 3.22) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(target_name example_basic)

project(${target_name})

file(GLOB GEN_SOURCES
    ../gen/objects/cube.cpp
    ../gen/shaders/object_vert.cpp
)

add_executable(${target_name} example_basic.cpp ${GEN_SOURCES})
target_include_directories(${target_name} PRIVATE ..)
target_link_libraries(${target_name} PRIVATE engine)

if(MSVC)
    copy_ext_dll(${target_name})
endif()

install(TARGETS ${target_name} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/deploy)