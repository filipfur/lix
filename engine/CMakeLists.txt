cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(LIX_ENGINE engine)

if(NOT DEFINED ENV{GLEW_HOME})
  message(FATAL_ERROR "GLEW_HOME is not set.")
else()
  set(GLEW_HOME $ENV{GLEW_HOME})
endif()
if(NOT DEFINED ENV{SDL2_HOME})
  message(FATAL_ERROR "SDL2_HOME is not set.")
else()
  set(SDL2_HOME $ENV{SDL2_HOME})
endif()

if(NOT DEFINED ASPROC_HOME)
  message(FATAL_ERROR "ASPROC_HOME is not set.")
endif()

if(NOT DEFINED GLM_HOME)
  message(FATAL_ERROR "GLM_HOME is not set.")
endif()

project(${LIX_ENGINE} VERSION 1.0.0 DESCRIPTION "Lightweight C++/OpenGL game engine.")
file(GLOB PROJ_SOURCES core/*.cpp render/*.cpp ecs/*.cpp format/*.cpp impact/*.cpp)
add_library(${LIX_ENGINE} STATIC ${PROJ_SOURCES})
set_target_properties(${LIX_ENGINE} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${LIX_ENGINE} PROPERTIES SOVERSION 1)
target_include_directories(${LIX_ENGINE} PUBLIC 
  ${GLEW_HOME}/include
  ${SDL2_HOME}/include
  core
  render
  ecs
  format
  impact
  ${GLM_HOME}
  ${ASPROC_HOME}/public
)
link_directories(
    ${GLEW_HOME}/lib
    ${SDL2_HOME}/lib
)

find_package(GLEW REQUIRED HINTS $ENV{GLEW_HOME})
find_package(SDL2 REQUIRED HINTS $ENV{SDL2_HOME})

set_target_properties(${LIX_ENGINE} PROPERTIES PUBLIC_HEADER include/*.h)

include(GNUInstallDirs)

if(MSVC)
  target_compile_options(${LIX_ENGINE} PRIVATE /W4 /WX)
  target_link_libraries(${LIX_ENGINE} PUBLIC json GLEW::GLEW SDL2::SDL2 opengl32)
else()
  target_compile_options(${LIX_ENGINE} PRIVATE -Wall -Wextra -Wpedantic -Werror)
  target_link_libraries(${LIX_ENGINE} PUBLIC json GLEW::GLEW SDL2::SDL2
        "-framework OpenGL")
endif()

#install(TARGETS ${LIX_ENGINE}
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
