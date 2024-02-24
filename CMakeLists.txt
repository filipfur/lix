cmake_minimum_required(VERSION 3.9)

set(PROJ lithium_x)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ASPROC_HOME asproc)

add_subdirectory(asproc)

project(${PROJ} VERSION 1.0.0 DESCRIPTION "Lightweight C++/OpenGL game engine.")
file(GLOB PROJ_SOURCES src/*.cpp impact/*.cpp ecs/*.cpp)
add_library(${PROJ} STATIC ${PROJ_SOURCES})
set_target_properties(${PROJ} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJ} PROPERTIES SOVERSION 1)
target_include_directories(${PROJ} PUBLIC include)
set_target_properties(${PROJ} PROPERTIES PUBLIC_HEADER include/*.h)

#include(GNUInstallDirs)

if(MSVC)
  target_compile_options(${PROJ} PRIVATE /W4 /WX)
  target_link_libraries(${PROJ} glew opengl32 SDL2)
else()
  target_compile_options(${PROJ} PRIVATE -Wall -Wextra -Wpedantic -Werror)
  target_link_libraries(${PROJ} glew SDL2
        "-framework OpenGL")
endif()

#install(TARGETS ${PROJ}
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

file(GLOB GEN_SOURCES gen/*/*.cpp)
add_executable(test_all test/test_all.cpp ${GEN_SOURCES})
add_executable(test_basic test/test_basic.cpp ${GEN_SOURCES})
target_include_directories(test_all PRIVATE .)
target_include_directories(test_basic PRIVATE .)
target_link_libraries(test_all ${PROJ})
target_link_libraries(test_basic ${PROJ})

install(TARGETS test_all test_basic DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/deploy)