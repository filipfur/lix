cmake_minimum_required(VERSION 3.22) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(target_name example_all)

project(${target_name})

file(GLOB GEN_SOURCES
    ../gen/shaders/anim_vert.cpp
    ../gen/shaders/object_vert.cpp
    ../gen/shaders/texture_object_adv_frag.cpp
    ../gen/shaders/inst_vert.cpp
    ../gen/shaders/screen_vert.cpp
    ../gen/shaders/screen_frag.cpp
    ../gen/shaders/blur_frag.cpp
    ../gen/shaders/bloom_frag.cpp
    ../gen/shaders/text_vert.cpp
    ../gen/shaders/text_wavy_vert.cpp
    ../gen/shaders/text_frag.cpp
    ../gen/shaders/text_border_frag.cpp
    ../gen/shaders/testtypes_vert.cpp
    ../gen/shaders/testtypes_frag.cpp
    ../gen/shaders/line_vert.cpp
    ../gen/shaders/line_frag.cpp
    ../gen/images/tex_png.cpp
    ../gen/objects/cube.cpp
    ../gen/objects/bush.cpp
    ../gen/objects/moose.cpp
    ../gen/objects/monkey_stage.cpp
    ../gen/objects/donut.cpp
    ../gen/objects/bone.cpp
    ../gen/objects/planets.cpp
    ../gen/objects/spikes.cpp
    ../gen/objects/animated_cube.cpp
    ../gen/fonts/arial.cpp
)

add_executable(${target_name} example_all.cpp ${GEN_SOURCES})
target_include_directories(${target_name} PRIVATE ..)
target_link_libraries(${target_name} PRIVATE engine)

if(MSVC)
    copy_ext_dll(${target_name})
endif()

install(TARGETS ${target_name} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/deploy)