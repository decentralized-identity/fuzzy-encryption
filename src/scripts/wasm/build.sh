#!/bin/bash

set -e
clear

export RED='\033[0;31m'
export GREEN='\033[0;32m'
export YELLOW='\033[1;33m'
export NC='\033[0m' # No Color

# print then run command
function run {
    printf "${YELLOW}$ ${1}${NC}\n"
    eval "${1}"
}

# print then run command but pipe stdout and stderr to null
function runq {
    printf "${YELLOW}$ ${1}${NC}\n"
    eval "${1} 1> /dev/null 2> /dev/null"
}

# print a comment in green
function comment {
    printf "${GREEN}${1}${NC}\n"
}

# create an empty directory even if it exists
function dir_create {
    if [ -d "$1" ]; then
        run "rm -r ${1}"
    fi
    run "mkdir -p ${1}"
}

function install {
    runq "which ${1}"
    if [ $? -ne 0 ]; then
        run "apt-get install ${1}"
    else
        comment "    ${1} detected"
    fi
}

export -f run
export -f runq
export -f comment
export -f dir_create
export -f install

export BUILD_PLATFORM=wasm
export OPENSSL_MINOR_VERSION=alpha10
export OPENSSL_DIR="/opt/fuzzy_openssl30_${OPENSSL_MINOR_VERSION}_${BUILD_PLATFORM}"
export FUZZY_LIB_WASM_DIR="/opt/fuzzy_lib_${BUILD_PLATFORM}"

export WASM_DIR=$PWD
cd ../../..
export ROOT_DIR=$PWD
cd $WASM_DIR

export WASM_OUTPUT_DIR=$ROOT_DIR/wasm_output_dir
export CMAKE_BUILD_DIR=$ROOT_DIR/build_$BUILD_PLATFORM
export PREREQ_BUILD_DIR=$ROOT_DIR/prereq_build_$BUILD_PLATFORM
export SRC_DIR=$ROOT_DIR/src
export SCRIPTS_DIR=$SRC_DIR/scripts/$BUILD_PLATFORM
export COMMON_SCRIPTS_DIR=$SRC_DIR/scripts/common
export HOST="linux-x86_64"
export CMAKE_VERSION=$(cmake --version | grep version | awk '{print $3}')
export TARGET_OPENSSL_SUBDIR="openssl-3.0.0-${OPENSSL_MINOR_VERSION}"

EMSCRIPTEN_DIR=$PREREQ_BUILD_DIR/emscripten_openssl
OPENSSL_SOURCE_DIR=$PREREQ_BUILD_DIR/$TARGET_OPENSSL_SUBDIR
EMSCRIPTEN_BIN_DIR=$PREREQ_BUILD_DIR/emsdk/upstream/emscripten
EMSCRIPTEN_OPENSSL_INCLUDE_DIR=$EMSCRIPTEN_DIR/system/include
EMSCRIPTEN_OPENSSL_LIB_DIR=$EMSCRIPTEN_DIR/system/lib

if [ -z $1 ]; then
    export BUILD_TYPE=Release
else
    if [ $1 == 'Release' ]; then
        export BUILD_TYPE=Release
    elif [ $1 == 'Debug' ]; then
        export BUILD_TYPE=Debug
    else
        printf "${RED}$1 is bogus${NC}\n"
        exit 1
    fi
fi

comment "This script takes about half an hour to complete"
comment "Here we go ..."
comment
comment "BUILD_PLATFORM                 ${BUILD_PLATFORM}"
comment "BUILD_TYPE                     ${BUILD_TYPE}"
comment "CMAKE_BUILD_DIR                ${CMAKE_BUILD_DIR}"
comment "CMAKE_VERSION                  ${CMAKE_VERSION}"
comment "COMMON_SCRIPTS_DIR             ${COMMON_SCRIPTS_DIR}"
comment "EMSCRIPTEN_BIN_DIR             ${EMSCRIPTEN_BIN_DIR}"
comment "EMSCRIPTEN_DIR                 ${EMSCRIPTEN_DIR}"
comment "EMSCRIPTEN_OPENSSL_LIB_DIR     ${EMSCRIPTEN_OPENSSL_LIB_DIR}"
comment "EMSCRIPTEN_OPENSSL_INCLUDE_DIR ${EMSCRIPTEN_OPENSSL_INCLUDE_DIR}"
comment "FUZZY_LIB_WASM_DIR             ${FUZZY_LIB_WASM_DIR}"
comment "HOST                           ${HOST}"
comment "OPENSSL_DIR                    ${OPENSSL_DIR}"
comment "OPENSSL_MINOR_VERSION          ${OPENSSL_MINOR_VERSION}"
comment "OPENSSL_SOURCE_DIR             ${OPENSSL_SOURCE_DIR}"
comment "ROOT_DIR                       ${ROOT_DIR}"
comment "PREREQ_BUILD_DIR               ${PREREQ_BUILD_DIR}"
comment "SCRIPTS_DIR                    ${SCRIPTS_DIR}"
comment "SRC_DIR                        ${SRC_DIR}"
comment "TARGET_OPENSSL_SUBDIR          ${TARGET_OPENSSL_SUBDIR}"
comment "WASM_DIR                       ${WASM_DIR}"
comment "WASM_OUTPUT_DIR                ${WASM_OUTPUT_DIR}"
comment

if [[ -f $COMMON_SCRIPTS_DIR/install_prereq.sh ]]; then
    run "bash $COMMON_SCRIPTS_DIR/install_prereq.sh"
    if [[ $? != 0 ]]; then
        printf "${RED}$COMMON_SCRIPTS_DIR/install_prereq.sh failed${NC}\n"
        exit $?
    fi
else
    printf "${RED}$COMMON_SCRIPTS_DIR/install_prereq.sh does not exist${NC}\n"
    exit 1
fi

dir_create $PREREQ_BUILD_DIR
dir_create $FUZZY_LIB_WASM_DIR
run "cd ${PREREQ_BUILD_DIR}"
runq "git clone https://github.com/emscripten-core/emsdk.git"
run "cd emsdk"
runq "git pull"
runq "./emsdk install latest"
runq "./emsdk activate latest"
runq ". ./emsdk_env.sh"
run "cd $PREREQ_BUILD_DIR"
runq "wget https://www.openssl.org/source/openssl-3.0.0-alpha10.tar.gz"
runq "tar xfvz ./openssl-3.0.0-alpha10.tar.gz"
dir_create $EMSCRIPTEN_OPENSSL_LIB_DIR
dir_create $EMSCRIPTEN_OPENSSL_INCLUDE_DIR
runq "cd $OPENSSL_SOURCE_DIR"
runq "emconfigure ./Configure linux-generic64 --prefix=$EMSCRIPTEN_DIR/system"
runq "sed -i 's|^CROSS_COMPILE.*$|CROSS_COMPILE=|g' Makefile"
runq "emmake make -j 12 build_generated libssl.a libcrypto.a"
runq "rm -rf $EMSCRIPTEN_DIR/system/include/openssl"
runq "cp -R $OPENSSL_SOURCE_DIR/include/openssl $EMSCRIPTEN_DIR/system/include/openssl"
runq "rm -rf $EMSCRIPTEN_DIR/system/include/crypto"
runq "cp -R $OPENSSL_SOURCE_DIR/include/openssl $EMSCRIPTEN_DIR/system/include/crypto"
runq "cp libcrypto.a libssl.a $EMSCRIPTEN_DIR/system/lib"
run "cd $ROOT_DIR/src/c++"

dir_create "${WASM_OUTPUT_DIR}"

printf "${YELLOW}$ ${EMSCRIPTEN_BIN_DIR}/emcc -v tests/loadrand/loadrand.cpp -o ${WASM_OUTPUT_DIR}/loadrand.js ... ${NC}\n"

${EMSCRIPTEN_BIN_DIR}/emcc -v \
  tests/loadrand/loadrand.cpp \
  -o ${WASM_OUTPUT_DIR}/loadrand.js \
  fuzzyvault/types.cpp \
  fuzzyvault/crypto.cpp \
  fuzzyvault/imod.cpp \
  fuzzyvault/matrix.cpp \
  fuzzyvault/poly.cpp \
  fuzzyvault/berlwelch.cpp \
  fuzzyvault/input.cpp \
  fuzzyvault/params.cpp \
  fuzzyvault/secret.cpp \
  fuzzyvault/fuzzy.cpp \
  fuzzyvault/parsing.cpp \
  fuzzyvault/utils.cpp \
  fuzzyvault/json.c \
  -Wno-deprecated-declarations \
  -I${EMSCRIPTEN_OPENSSL_INCLUDE_DIR} \
  -I${ROOT_DIR}/src/c++/fuzzyvault \
  -L${EMSCRIPTEN_OPENSSL_LIB_DIR} \
  -lssl \
  -lcrypto \
  -s USE_PTHREADS=1 \
  -s EXIT_RUNTIME=1 \
  -s DISABLE_EXCEPTION_CATCHING=0 \
  -DWASM 1> /dev/null 2> /dev/null

if [[ $? != 0 ]]; then
    printf "${RED}build failed${NC}\n"
    exit 2
fi

if [[ $? != 0 ]]; then
    printf "${RED}build failed${NC}\n"
    exit 3
fi

runq "cd ${WASM_OUTPUT_DIR}"
run "node --experimental-wasm-threads --experimental-wasm-bulk-memory loadrand.js"

if [[ $? != 0 ]]; then
    printf "${RED}loadrand failed${NC}\n"
    exit 4
else
    comment "build verification test passed"
fi

exit 0
