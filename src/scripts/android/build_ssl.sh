#!/bin/bash
#!/bin/sh

set -e
comment "build_ssh.sh"

OPENSSL_DIR="${PWD}/openssl"

runq "rm -rf openssl"
run "git clone https://github.com/openssl/openssl.git"
runq "cd ${OPENSSL_DIR}"
runq "git checkout tags/openssl-3.0.0-${OPENSSL_MINOR_VERSION}"

# ARGUMENTS 
# Set this to your minSdkVersion.
export API=$1
export BUILD=$2

if [ -z $API ]; then
    warning "No API version was provided"
    exit 1
fi
# androidtoolsdir
# /opt/android-sdk

# Please refer to https://developer.android.com/ndk/guides/other_build_systems#autoconf for documentation

NDK=/opt/android-sdk/ndk/android-ndk-r21d
export ANDROID_NDK_ROOT=$NDK
export ANDROID_NDK_HOME=$NDK
HOST="linux-x86_64"

export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/$HOST
# export SYSROOT=$TOOLCHAIN/sysroot
export PATH=$TOOLCHAIN/bin:$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/$HOST/bin:$PATH

function buildLibraries {
    abi=$1
    comment "build_ssl.buildLibraries $abi"
    outputFolder=""
    if [[ $abi == "android-arm" ]]; then
        export TARGET=armv7a-linux-androideabi
        outputFolder="armeabi-v7a"
    elif [[ $abi == "android-arm64" ]]; then
        export TARGET=aarch64-linux-android
        outputFolder="arm64-v8a"
    elif [[ $abi == "android-x86" ]]; then
        export TARGET=i686-linux-android
        outputFolder="x86"
    elif [[ $abi == "android-x86_64" ]]; then
        export TARGET=x86_64-linux-android
        outputFolder="x86_64"
    else 
        warning "Unknown ABI: $abi"
        return
    fi

    # Configure and build.
    # export AR=$TOOLCHAIN/bin/$TARGET-ar
    # export AS=$TOOLCHAIN/bin/$TARGET-as
    # export CC=$TOOLCHAIN/bin/$TARGET$API-clang
    # export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
    # export LD=$TOOLCHAIN/bin/$TARGET-ld
    # export RANLIB=$TOOLCHAIN/bin/$TARGET-ranlib
    # export STRIP=$TOOLCHAIN/bin/$TARGET-strip
    # export NM=$TOOLCHAIN/bin/$TARGET-nm

    debugArg=""
    if [[ $(echo "$BUILD" | tr '[:upper:]' '[:lower:]') == d*  ]]; then
        debugArg="-d"
    fi
    
    runq "./Configure $abi -D__ANDROID_API__="$API" $debugArg shared no-asm SYSROOT="$TOOLCHAIN/sysroot" -fstack-protector-strong"
    runq "make clean"
    runq "make update"
    runq "git clean -f"
    runq "make"

    comment "Build successful."

    runq "mkdir -p ${ANDROID}/$outputFolder/lib"
    runq "cp ./*.so ${ANDROID}/$outputFolder/lib"
    runq "cp ./*.a ${ANDROID}/$outputFolder/lib"
    runq "cp -r ./include ${ANDROID}/$outputFolder/"

    # runq "make clean"
}

runq "cd ${OPENSSL_DIR}"

buildLibraries android-arm
buildLibraries android-x86_64
buildLibraries android-arm64
buildLibraries android-x86