cmake_minimum_required(VERSION 3.22) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(asproc)

include_directories(
    include
    ../json
    ../gltf
    ../ttf
)

file(GLOB SOURCES
    src/asproc.cpp
    src/common.cpp
    src/imageproc.cpp
    src/objectproc.cpp
    src/fontproc.cpp
    src/shaderproc.cpp
    ../json/json.cpp
)

add_executable(asproc ${SOURCES})

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)