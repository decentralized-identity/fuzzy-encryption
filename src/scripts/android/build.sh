#!/bin/bash
#!/bin/sh

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

function warning {
    printf "${RED}${1}${NC}\n"
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
export -f warning


export API=$1
export BUILD=$2

if [ -z $API ]; then
    warning "No API version was provided, eg. 23"
    exit 1
fi

if [ -z $BUILD ]; then
    warning "No BUILD platform was provided, eg. Release or Debug"
    exit 1
fi


export ANDROID_DIR=$PWD
export OPENSSL_DIR="${PWD}/openssl"
export OPENSSL_MINOR_VERSION=alpha8
export NDK=/opt/android-sdk/ndk/android-ndk-r21d
export ANDROID_NDK_ROOT=$NDK
export ANDROID_NDK_HOME=$NDK
export HOST="linux-x86_64"
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/$HOST
export PATH=$TOOLCHAIN/bin:$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/$HOST/bin:$PATH
export ANDROID_LIB_DIR=/opt/fuzzy_lib_android
export ANDROID_OPENSSL_DIR="${PWD}/openssl"
export NDK_DIR=/opt/android-sdk/ndk

cd "../../.."
export BUILD_SOURCEDIRECTORY=$PWD
cd $ANDROID_DIR

comment
comment "API                    ${API}"
comment "BUILD                  ${BUILD}"
comment "ANDROID_DIR            ${ANDROID_DIR}"
comment "OPENSSL_DIR            ${OPENSSL_DIR}"
comment "OPENSSL_MINOR_VERSION  ${OPENSSL_MINOR_VERSION}"
comment "NDK                    ${NDK}"
comment "ANDROID_NDK_ROOT       ${ANDROID_NDK_ROOT}"
comment "ANDROID_NDK_HOME       ${ANDROID_NDK_HOME}"
comment "TOOLCHAIN              ${TOOLCHAIN}"
comment "HOST                   ${HOST}"
comment "PATH                   ${PATH}"
comment "TOOLCHAIN              ${TOOLCHAIN}"
comment "PATH                   ${PATH}"
comment "ANDROID_LIB_DIR        ${ANDROID_LIB_DIR}"
comment "ANDROID_OPENSSL_DIR    ${ANDROID_OPENSSL_DIR}"
comment "NDK_DIR                ${NDK_DIR}"
comment "BUILD_SOURCEDIRECTORY  ${BUILD_SOURCEDIRECTORY}"


bash install_ndk.sh
bash build_ssl.sh $API $BUILD
bash build_android.sh $API $BUILD
