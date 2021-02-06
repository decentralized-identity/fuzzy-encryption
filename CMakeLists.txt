# Copyright (c) Microsoft Corporation.
# Licensed under the MIT license.

cmake_minimum_required (VERSION 3.13.4)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(RELEASE_CFLAGS "-O3 -mtune=intel -DNDEBUG" CACHE STRING "C compiler options for release builds")
set(CMAKE_C_FLAGS_DEBUG "-Og -D_FORTIFY_SOURCE -fstack-protector-strong")
set(CMAKE_C_FLAGS_RELEASE ${RELEASE_CFLAGS})
set(CMAKE_CXX_FLAGS_RELEASE ${RELEASE_CFLAGS})

# default CFLAGS
set(COMMON_CFLAGS "-g3 -fPIC -Wno-deprecated-declarations -Wall -Wextra -Wno-uninitialized -Wno-unused-parameter -Wl,-zdefs -fvisibility=hidden")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_CFLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${COMMON_CFLAGS} -std=c++11 -D__STDC_LIMIT_MACROS")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/.
)

add_subdirectory(src)
