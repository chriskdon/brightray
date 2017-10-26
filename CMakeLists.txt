cmake_minimum_required (VERSION 2.8)
cmake_policy(VERSION 2.8)
project(libbrightray)

include_directories (${PROJECT_SOURCE_DIR}/src/)

SET(LIBPATH /usr/lib /usr/local/lib)

project (brightray)

add_subdirectory(src)
add_subdirectory(examples)