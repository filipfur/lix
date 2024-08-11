cmake_minimum_required(VERSION 3.22) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(target_name example_platformer)

project(${target_name})

file(GLOB GEN_SOURCES
    ../gen/objects/cube.cpp
    ../gen/objects/spyro_ps1.cpp
    ../gen/objects/macman.cpp
    ../gen/objects/lilbro.cpp
    ../gen/objects/rat.cpp
    ../gen/objects/donut.cpp
    ../gen/objects/bridge.cpp
    ../gen/objects/level.cpp
    ../gen/objects/home.cpp
    ../gen/objects/platform.cpp
    ../gen/fonts/josefin_sans.cpp
    ../gen/shaders/object_vert.cpp
)

add_executable(${target_name} example_platformer.cpp ${GEN_SOURCES})
target_include_directories(${target_name} PRIVATE ..)
target_link_libraries(${target_name} PRIVATE engine)

if(MSVC)
    copy_ext_dll(${target_name})
endif()

install(TARGETS ${target_name} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/deploy)