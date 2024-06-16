cmake_minimum_required(VERSION 3.22) 

set(target_name lix_unit_test)

project(${target_name})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()
include(CTest)

add_executable(${target_name} unit_test_impact.cpp)
target_include_directories(${target_name} PRIVATE ..)
target_link_libraries(${target_name} engine)

if(MSVC)
    copy_ext_dll(${target_name})
endif()

# define tests
add_test(
  NAME ${target_name}
  COMMAND $<TARGET_FILE:${target_name}>
  )