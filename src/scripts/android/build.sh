#!/bin/sh

# ARGUMENTS 
# Set this to your minSdkVersion.
export API=$1
export BUILD=$2

if [ -z $API ]; then
    echo "No API version was provided"
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
    echo "Building $abi..."
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
        echo "Unknown ABI: $abi"
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
    
    ./Configure $abi -D__ANDROID_API__="$API" $debugArg shared no-asm SYSROOT="$TOOLCHAIN/sysroot" -fstack-protector-strong
    make clean
    make update # needed to update libcrypto.map number bindings
    git clean -f
    make

    if [[ $? != 0 ]]; then
        echo "[ERROR] Make failed"
        exit 1
    fi

    echo "Build successful."

    mkdir -p /opt/fuzzy-lib/$outputFolder/lib
    cp ./*.so /opt/fuzzy-lib/$outputFolder/lib
    cp ./*.a /opt/fuzzy-lib/$outputFolder/lib
    cp -r ./include /opt/fuzzy-lib/$outputFolder/

    make clean
}
cd openssl

buildLibraries android-arm
buildLibraries android-x86_64
buildLibraries android-arm64
buildLibraries android-x86