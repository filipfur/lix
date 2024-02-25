cmake_minimum_required(VERSION 3.22) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ASPROC_HOME ${CMAKE_CURRENT_SOURCE_DIR}/asproc)

project(lix)

add_subdirectory(glm/glm)
add_subdirectory(json)
add_subdirectory(asproc)
add_subdirectory(engine)
add_subdirectory(test)