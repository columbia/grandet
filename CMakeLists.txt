cmake_minimum_required (VERSION 3.1)
project (grandet)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O4")
set(CMAKE_CXX_FLAGS_PROFILE "-pg -O4 -g")

include_directories(common)
include_directories(include)

find_package (PkgConfig)
pkg_check_modules (REDIS3M REQUIRED redis3m)
include_directories(${REDIS3M_INCLUDE_DIRS})
link_directories(${REDIS3M_STATIC_LIBRARY_DIRS})

find_library (LIBAWS_LIBRARY NAMES aws)

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_subdirectory (decider)
add_subdirectory (actor)
add_subdirectory (proto)
add_subdirectory (estimator)
add_subdirectory (common)
add_subdirectory (server)
add_subdirectory (profiler)
add_subdirectory (console)
add_subdirectory (sdk)
add_subdirectory (bench)
