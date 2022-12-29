cmake_minimum_required (VERSION 3.22)
project (PROJECT_NAME_GOES_HERE VERSION 1.0.0 LANGUAGES CXX)

# add support for CTest
include(CTest)
include(FetchContent)
include(ExternalProject)

set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS " \
    -Wno-parentheses \
    -Werror \
    -Wall \
    -Wextra \
    -pedantic-errors \
    -O2 \
    -finline-functions \
    -D PROJECT_DIR=\"\\\"${CMAKE_SOURCE_DIR}\\\"\"")

# set (CMAKE_VERBOSE_MAKEFILE on)

set(EXTERNAL_INSTALL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/external)

set (CMAKE_INSTALL_PREFIX /base_dir)

INSTALL (
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/dir
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
        FILES_MATCHING PATTERN "*.h"
)

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/UnitTest.cmake)

# Recurse subdirectories
add_subdirectory (src)
add_subdirectory (test)
