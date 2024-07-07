cmake_minimum_required(VERSION 3.22) 

project(lix_examples)

if(NOT DEFINED ASPROC_HOME)
  message(FATAL_ERROR "ASPROC_HOME is not set.")
endif()

if(WIN32)
  message(STATUS "Make not a thing on windows.")
else()
  add_custom_target(
      GenerateAssets ALL
      COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR} -f ${ASPROC_HOME}/gen_assets.mk gen_assets ASPROC_HOME=${ASPROC_HOME}
  )
  add_dependencies(GenerateAssets asproc)
endif()

add_subdirectory(example_all)
add_subdirectory(example_basic)
add_subdirectory(example_collision)