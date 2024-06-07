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
    src/plyproc.cpp
    src/gltfexport.cpp
)

add_executable(asproc ${SOURCES})

target_link_libraries(asproc json)

target_include_directories(asproc PRIVATE
    include
    ${GLM_HOME}
)

target_include_directories(asproc PUBLIC
    public
)

set_target_properties(asproc PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin
)
install(TARGETS asproc DESTINATION bin)