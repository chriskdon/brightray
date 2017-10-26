cmake_minimum_required (VERSION 2.8)
cmake_policy(VERSION 2.8)
project(libbrightray)

SET(CMAKE_C_FLAGS "-Wall -Werror")

include_directories (${PROJECT_SOURCE_DIR}/src/)

add_subdirectory(src)
add_subdirectory(examples)