# Copyright (c) Microsoft Corporation.
# Licensed under the MIT license.

# project name
project(randomtest)

# demo target
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

include_directories(${CMAKE_CURRENT_LIST_DIR}/../../fuzzyvault)

find_package(OpenSSL REQUIRED) 
if( OpenSSL_FOUND )
    include_directories(${OPENSSL_INCLUDE_DIRS})
    link_directories(${OPENSSL_LIBRARIES})
    message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
endif()

# Create shared library
# target_link_libraries(${PROJECT_NAME} fuzzyvault)

ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} -L${CMAKE_CURRENT_SOURCE_DIR}/../../../../build/src/c++ fuzzyvault)
