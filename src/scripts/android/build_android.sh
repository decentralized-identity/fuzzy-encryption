#!/bin/bash
#!/bin/sh

comment
comment "    build_android.sh"
comment

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
    comment
    comment "   build_android.buildLibraries $abi"
    comment
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

    runq "cd ${BUILD_SOURCEDIRECTORY}"
    runq "rm -r build_android"
    runq "mkdir build_android"
    runq "cd build_android"
    
    runq "mkdir -p ${ANDROID_LIB_DIR}/$outputFolder/lib"
    runq "mkdir -p ${ANDROID_LIB_DIR}/$outputFolder/include" 

    local ARG1="-DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake"
    local ARG2="-DANDROID_ABI=${outputFolder}"
    local ARG3="-DANDROID_NATIVE_API_LEVEL=${API}"
    local ARG4="-DCMAKE_BUILD_TYPE=${BUILD}"
    local ARG5="-DOPENSSL_CRYPTO_LIBRARY=${ANDROID_OPENSSL_DIR}/${outputFolder}/lib"
    local ARG6="-DOPENSSL_INCLUDE_DIR=${ANDROID_OPENSSL_DIR}/${outputFolder}/include"
    local ARG7="--config ${BUILD}"
    local ARG8="-B."
    local ARG9="-S.."
    local ARGS="${ARG1} ${ARG2} ${ARG3} ${ARG4} ${ARG5} ${ARG6} ${ARG7} ${ARG8} ${ARG9}"
    
    runq "cmake $ARGS"

    # runq "cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake -DANDROID_ABI=$outputFolder -DANDROID_NATIVE_API_LEVEL=$API -DCMAKE_BUILD_TYPE=$BUILD -DOPENSSL_CRYPTO_LIBRARY=${ANDROID_OPENSSL_DIR}/$outputFolder/lib -DOPENSSL_INCLUDE_DIR=${ANDROID_OPENSSL_DIR}/$outputFolder/include --config $BUILD -B. -S.."
    
    runq make

    if [[ $? != 0 ]]; then
        warning "[ERROR] Make failed"
        exit 1
    fi

    runq "cp ./src/c++/fuzzyvault/*.so ${ANDROID_LIB_DIR}/$outputFolder/lib"
    runq "cp ./src/c++/fuzzyvault/*.a ${ANDROID_LIB_DIR}/$outputFolder/lib"
    runq "cp ../src/c++/fuzzyvault/fuzzy.h ${ANDROID_LIB_DIR}/$outputFolder/include/"

    runq "cd .."
}
cd $BUILD_SOURCESDIRECTORY

buildLibraries android-arm
buildLibraries android-x86_64
buildLibraries android-arm64
buildLibraries android-x86
