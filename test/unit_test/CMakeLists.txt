cmake_minimum_required(VERSION 3.22) 

project(lix_unit_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()
include(CTest)

add_executable(lix_unit_test unit_test_impact.cpp)
target_include_directories(lix_unit_test PRIVATE ..)
target_link_libraries(lix_unit_test engine)

# define tests
add_test(
  NAME lix_unit_test
  COMMAND $<TARGET_FILE:lix_unit_test>
  )