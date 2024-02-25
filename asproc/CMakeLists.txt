cmake_minimum_required(VERSION 3.22) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(asproc)

file(GLOB SOURCES
    src/asproc.cpp
    src/common.cpp
    src/imageproc.cpp
    src/objectproc.cpp
    src/fontproc.cpp
    src/shaderproc.cpp
)

add_executable(asproc ${SOURCES})

target_link_libraries(asproc json)

target_include_directories(asproc PRIVATE
    include
    ${CMAKE_SOURCE_DIR}/glm
)

target_include_directories(asproc PUBLIC
    public
)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

install(TARGETS asproc DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)