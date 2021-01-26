# Copyright (c) Microsoft Corporation.
# Licensed under the MIT license.

# project name
project(fuzzyvault)

# fuzzylib target
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

include_directories(${CMAKE_CURRENT_LIST_DIR})

find_package(OpenSSL REQUIRED) 
if( OpenSSL_FOUND )
    include_directories(${OPENSSL_INCLUDE_DIR})
    link_directories(${OPENSSL_LIBRARIES})
    message(STATUS "OPENSSL_LIBRARIES ${OPENSSL_LIBRARIES}")
    message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
endif()

# Create shared library
add_library(fuzzy-objs OBJECT ${SOURCES})
add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:fuzzy-objs>)
add_library(${PROJECT_NAME}-static STATIC $<TARGET_OBJECTS:fuzzy-objs>)
set_target_properties(${PROJECT_NAME}-static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} ssl crypto)

