#!/bin/bash

set -e
comment "build_ssl.h"

install "wget"
install "unzip"
install "tar"
dir_create "$PREREQ_BUILD_DIR"
run "cd ${PREREQ_BUILD_DIR}"

# runq "wget https://www.openssl.org/source/${TARGET_SUBDIR}.tar.gz"
# runq "tar xfz ./${TARGET_SUBDIR}.tar.gz"

runq "rm -rf openssl"
run "git clone https://github.com/openssl/openssl.git"
runq "cd openssl"
runq "git checkout tags/openssl-3.0.0-${OPENSSL_MINOR_VERSION}"


# run "cd ${TARGET_SUBDIR}"
# dir_create $OPENSSL_DIR
runq "./Configure --prefix=$OPENSSL_DIR --openssldir=$OPENSSL_DIR -fstack-protector-strong"
runq "make clean"
runq "make update"
runq "make"
if [ $? -ne 0 ]; then
    printf "${RED}[ERROR] Make failed${NC}\n"
    exit 1
fi
# skipping make test
# run "make test"
# if [ $? -ne 0 ]; then
#     echo "[ERROR] Make failed"
#     exit 1
# fi
runq "make install"
if [ $? -ne 0 ]; then
    printf "${RED}[ERROR] Make failed${NC}\n"
    exit 1
fi
