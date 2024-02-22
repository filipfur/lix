cmake_minimum_required(VERSION 3.9)

set(LIX lithium_x)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ASPROC_HOME asproc)

add_subdirectory(asproc)

project(${LIX} VERSION 1.0.0 DESCRIPTION "Lightweight C++/OpenGL game engine.")

file(GLOB LIX_SOURCES src/*.cpp impact/*.cpp ecs/*.cpp)

add_library(${LIX} STATIC ${LIX_SOURCES})

set_target_properties(${LIX} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${LIX} PROPERTIES SOVERSION 1)

target_include_directories(${LIX} PUBLIC include)
set_target_properties(${LIX} PROPERTIES PUBLIC_HEADER include/*.h)

include(GNUInstallDirs)

if (WIN32) 
    target_link_libraries(${LIX} glew opengl32)
else ()
    target_link_libraries(${LIX} glew
        "-framework OpenGL")
endif ()

#install(TARGETS ${LIX}
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


file(GLOB GEN_SOURCES gen/*/*.cpp)
add_executable(test_all test/test_all.cpp ${GEN_SOURCES})
target_include_directories(test_all PRIVATE .)
target_link_libraries(test_all ${LIX} SDL2 glew)

install(TARGETS test_all DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/deploy)