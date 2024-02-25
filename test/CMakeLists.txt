cmake_minimum_required(VERSION 3.22) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(lix_test)

if(NOT DEFINED ASPROC_HOME)
  message(FATAL_ERROR "ASPROC_HOME is not set.")
endif()

add_custom_target(
    GenerateAssets ALL
    COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR} -f ${ASPROC_HOME}/gen_assets.mk gen_assets ASPROC_HOME=${ASPROC_HOME}
)

add_subdirectory(test_all)
add_subdirectory(test_basic)