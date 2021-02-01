#!/bin/bash

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

# print then run command but pipe stdout and stderr to null
function runqs {
    printf "${YELLOW}$ ${1}${NC}\n"
    eval "sudo ${1} 1> /dev/null 2> /dev/null"
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
        run "apt-get install ${1} -y"
    else
        comment "    ${1} detected"
    fi
}

export -f run
export -f runq
export -f comment
export -f dir_create
export -f install

export LINUX_DIR=$PWD
cd ../../..
export ROOT_DIR=$PWD
cd $LINUX_DIR


# printf "${RED}WARNING${NC}\n"
# comment 
# comment "  This script may run some of the following commands."
# comment
# comment "       apt-get update"
# comment "       apt-get upgrade"
# comment "       apt-get dist-upgrade"
# comment "       apt-get install man"
# comment "       apt-get install wget"
# comment "       apt-get install perl"
# comment "       apt-get install buid-essential"
# comment "       apt-get install git"
# comment "       apt-get install python3"
# comment "       apt-get install cmake"
# comment "       apt-get install python3-pip"
# comment "       apt-get install default-jre"
# comment "       apt-get install unzip"
# comment "       apt-get install npm"
# comment "       apt-get install node"
# comment "       apt-get purge cmake"
# comment "       snap install cmake --classic"
# comment "       . ./emsdk_env.sh"
# comment "       npm install -g n"
# comment "       n stable"
# comment
# read -p "Continue? [yes|no] "
# if [ "$REPLY" != "yes" ]; then
#     exit
# fi
# echo

export BUILD_PLATFORM=linux
export OPENSSL_MINOR_VERSION=alpha8
export OPENSSL_DIR="/opt/fuzzy_openssl30_${OPENSSL_MINOR_VERSION}_${BUILD_PLATFORM}"
export FUZZY_LIB_DIR="/opt/fuzzy-lib_${BUILD_PLATFORM}"
export CMAKE_BUILD_DIR=$ROOT_DIR/build_$BUILD_PLATFORM
export PREREQ_BUILD_DIR=$ROOT_DIR/prereq_build_$BUILD_PLATFORM
export SRC_DIR=$ROOT_DIR/src
export SCRIPTS_DIR=$SRC_DIR/scripts/$BUILD_PLATFORM
export COMMON_SCRIPTS_DIR=$SRC_DIR/scripts/common
export HOST="linux-x86_64"
export CMAKE_VERSION=$(cmake --version | grep version | awk '{print $3}')
export TARGET_SUBDIR="openssl-3.0.0-${OPENSSL_MINOR_VERSION}"

if [ -z $1 ]; then 
    export BUILD_TYPE=Release
else
    if [ $1 == 'Release' ]; then
        export BUILD_TYPE=Release
    elif [ $1 == 'Debug' ]; then
        export BUILD_TYPE=Debug
    else
        printf "${RED}${1} is bogus${NC}\n"
        exit 1
    fi
fi

printf "${YELLOW}This script takes up to two hours to complete on my machine. Be patient ...${NC}\n"
comment
comment "script variable values:"
comment
comment "BUILD_PLATFORM           ${BUILD_PLATFORM}"
comment "BUILD_TYPE               ${BUILD_TYPE}"
comment "CMAKE_BUILD_DIR          ${CMAKE_BUILD_DIR}"
comment "CMAKE_VERSION            ${CMAKE_VERSION}"
comment "COMMON_SCRIPTS_DIR       ${COMMON_SCRIPTS_DIR}"
comment "FUZZY_LIB_DIR            ${FUZZY_LIB_DIR}"
comment "HOST                     ${HOST}"
comment "LINUX_DIR                ${LINUX_DIR}"
comment "OPENSSL_DIR              ${OPENSSL_DIR}"
comment "OPENSSL_MINOR_VERSION    ${OPENSSL_MINOR_VERSION}"
comment "ROOT_DIR                 ${ROOT_DIR}"
comment "PREREQ_BUILD_DIR         ${PREREQ_BUILD_DIR}"
comment "SCRIPTS_DIR              ${SCRIPTS_DIR}"
comment "SRC_DIR                  ${SRC_DIR}"
comment "TARGET_SUBDIR            ${TARGET_SUBDIR}"
comment
comment "here we go ..."
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

if [[ -f $LINUX_DIR/build_ssl.sh ]]; then
    run "bash $LINUX_DIR/build_ssl.sh"
    if [[ $? != 0 ]]; then
        printf "${RED}$LINUX_DIR/build_ssl.h failed${NC}\n"
        exit $?
    fi
else
    printf "${RED}$LINUX_DIR/build_ssh.sh does not exist${NC}\n"
    exit 2
fi

if [[ -f $LINUX_DIR/azure-linux-build.sh ]]; then
    run "bash $LINUX_DIR/azure-linux-build.sh"
    if [[ $? != 0 ]]; then
        printf "${RED}$LINUX_DIR/azure-linux-build.sh failed${NC}\n"
        exit $?
    fi
else
    printf "${RED}$LINUX_DIR/azure-linux-build.sh does not exist${NC}\n"
    exit 3
fi

comment "build verification test passed"
