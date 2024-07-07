cmake_minimum_required(VERSION 3.22) 

project(lix_unit_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(lix_add_test target_name)
  add_executable(${target_name} unit_test.cpp ${target_name}.cpp)
  target_link_libraries(${target_name} engine)
  add_test(NAME ${target_name} COMMAND $<TARGET_FILE:${target_name}>)
endfunction()

lix_add_test(test_time)