#!/bin/bash
#!/bin/sh

# ARGUMENTS 
# Set this to your minSdkVersion.
export API=$1
export BuildType=$2

if [ -z $BUILD_SOURCESDIRECTORY ]; then 
    BUILD_SOURCESDIRECTORY='../../..'
fi

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

    rm -r build-android
    mkdir build-android
    cd build-android

    cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
        -DANDROID_ABI=$outputFolder \
        -DANDROID_NATIVE_API_LEVEL=$API \
        -DCMAKE_BUILD_TYPE=$BuildType \
        -DOPENSSL_CRYPTO_LIBRARY=/opt/fuzzy-lib/$outputFolder/lib \
        -DOPENSSL_INCLUDE_DIR=/opt/fuzzy-lib/$outputFolder/include \
        --config $BuildType -B. -S..
    make

    if [[ $? != 0 ]]; then
        echo "[ERROR] Make failed"
        exit 1
    fi

    echo "Build successful."

    cp ./src/c++/fuzzyvault/*.so /opt/fuzzy-lib/$outputFolder/lib
    cp ./src/c++/fuzzyvault/*.a /opt/fuzzy-lib/$outputFolder/lib
    cp ../src/c++/fuzzyvault/fuzzy.h /opt/fuzzy-lib/$outputFolder/include/

    cd ..
}
cd $BUILD_SOURCESDIRECTORY

buildLibraries android-arm
buildLibraries android-x86_64
buildLibraries android-arm64
buildLibraries android-x86