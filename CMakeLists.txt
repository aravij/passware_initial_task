cmake_minimum_required(VERSION 3.6)
project(test_problem)

# adding extra modules and scripts for GCrypt and functional testing
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")
list(APPEND CMAKE_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/cmake/Scripts/")

add_subdirectory(src)
add_subdirectory(test)
