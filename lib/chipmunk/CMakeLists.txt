cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0001 NEW) # don't use MAKE_BACKWARDS_COMPATIBILITY but policies instead

project(chipmunk)

set(CMAKE_BUILD_TYPE Release CACHE INTERNAL
    "set to 'Release' so CP doesn't print to stdout" FORCE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -ffast-math -Wall")

add_subdirectory(src)

