cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(LIX_ENGINE engine)

if(NOT DEFINED GLEW_HOME)
    if(NOT DEFINED ENV{GLEW_HOME})
        message(FATAL_ERROR "GLEW_HOME is not set.")
    else()
        set(GLEW_HOME $ENV{GLEW_HOME})
    endif()
endif()

if(NOT DEFINED SDL2_HOME)
    if(NOT DEFINED ENV{SDL2_HOME})
        message(FATAL_ERROR "SDL2_HOME is not set.")
    else()
        set(SDL2_HOME $ENV{SDL2_HOME})
    endif()
endif()

if(NOT DEFINED ASPROC_HOME)
    message(FATAL_ERROR "ASPROC_HOME is not set.")
endif()

if(NOT DEFINED GLM_HOME)
    message(FATAL_ERROR "GLM_HOME is not set.")
endif()

project(${LIX_ENGINE} VERSION 1.0.0 DESCRIPTION "Lightweight C++/OpenGL game engine.")
file(GLOB PROJ_SOURCES core/*.cpp render/*.cpp ecs/*.cpp format/*.cpp impact/*.cpp feature/*.cpp)
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
    feature
    utility
    ${GLM_HOME}
    ${ASPROC_HOME}/public
)
target_link_directories(${LIX_ENGINE} PUBLIC
    ${GLEW_HOME}/lib
    ${SDL2_HOME}/lib
)

set_target_properties(${LIX_ENGINE} PROPERTIES
    PUBLIC_HEADER include/*.h
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(GNUInstallDirs)
if(EMSCRIPTEN)
    message(INFO "CONFIGURATION: EMSCRIPTEN")
    target_compile_options(${LIX_ENGINE} PUBLIC
        -Wall
        -Wextra
        -O3
    )
    target_link_options(${LIX_ENGINE} PUBLIC
        "-s FULL_ES3=1"
        "-s USE_WEBGL2=1"
        "-s MIN_WEBGL_VERSION=2"
        "-s USE_SDL=2"
        "-s ALLOW_MEMORY_GROWTH=1"
        "-s INITIAL_MEMORY=33554432 -s STACK_SIZE=16777216"
    )
elseif(APPLE)
    message(INFO "CONFIGURATION: APPLE")
    find_package(GLEW REQUIRED HINTS $ENV{GLEW_HOME})
    find_package(SDL2 REQUIRED HINTS $ENV{SDL2_HOME})
    target_compile_options(${LIX_ENGINE} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_link_libraries(${LIX_ENGINE} PUBLIC json GLEW::GLEW SDL2::SDL2
        "-framework OpenGL")
elseif(MSVC)
    message(INFO "CONFIGURATION: MSVC")
    SET(GLEW_DLL "${GLEW_HOME}/bin/glew32.dll" CACHE INTERNAL "GLEW_DLL")
    SET(SDL2_DLL "${SDL2_HOME}/bin/SDL2.dll" CACHE INTERNAL "SDL2_DLL")
    function(copy_ext_dll target_name)
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GLEW_DLL}"
            $<TARGET_FILE_DIR:${target_name}>)
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL2_DLL}"
            $<TARGET_FILE_DIR:${target_name}>)
    endfunction()
    target_compile_options(${LIX_ENGINE} PRIVATE /W4 /WX)
    target_link_libraries(${LIX_ENGINE} PUBLIC json glew32s SDL2Main SDL2 opengl32)
else() # linux
    message(INFO "CONFIGURATION: OTHER")
    target_link_libraries(${LIX_ENGINE} PUBLIC json glew32 SDL2Main SDL2 opengl32)
endif()

# install(TARGETS ${LIX_ENGINE}
# LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
# PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
