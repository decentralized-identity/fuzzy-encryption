#!/bin/bash

if [[ ${CMAKE_VERSION} < "3.13.4" ]]; then
    run "sudo apt-get purge cmake"
    run "sudo snap install cmake --classic"

    if [[ $? != 0 ]]; then
        printf "${RED}failed to upgrade cmake${NC}\n"
        exit 2
    fi
    CMAKE_VERSION=$(cmake --version | grep version | awk '{print $3}')
    printf "${GREEN}CMAKE_VERSION            ${CMAKE_VERSION}${NC}\n"
fi
dir_create $CMAKE_BUILD_DIR
run "cd $CMAKE_BUILD_DIR"
runq "cmake -DOPENSSL_CRYPTO_LIBRARY=$OPENSSL_DIR/lib -DOPENSSL_INCLUDE_DIR=$OPENSSL_DIR/include -S.."
if [[ $? != 0 ]]; then
    printf "${RED}[ERROR] cmake failed${NC}\n"
    exit 1
fi
runq "make"
if [[ $? != 0 ]]; then
    printf "${RED}[ERROR] make failed${NC}\n"
    exit 1
fi
dir_create $FUZZY_LIB_DIR
run "mkdir -p ${FUZZY_LIB_DIR}/include"
run "mkdir -p ${FUZZY_LIB_DIR}/bin"
run "mkdir -p ${FUZZY_LIB_DIR}/lib"
run "cp ./src/c++/fuzzyvault/*.so $FUZZY_LIB_DIR/lib"
run "cp ./src/c++/fuzzyvault/*.a $FUZZY_LIB_DIR/lib"
run "cp $SRC_DIR/c++/fuzzyvault/fuzzy.h $FUZZY_LIB_DIR/include/"
run "cp ./src/c++/tests/demo/demo $FUZZY_LIB_DIR/bin/"
run "cp ./src/c++/tests/randomtest/randomtest $FUZZY_LIB_DIR/bin/"
run "cd $FUZZY_LIB_DIR/bin"
run "./demo"
if [[ $? != 0 ]]; then
    printf "${RED}[ERROR] Failed running build verfication test${NC}\n"
    exit 1
fi
