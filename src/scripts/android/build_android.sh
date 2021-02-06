#!/bin/bash
#!/bin/sh

comment
comment "    build_android.sh"
comment

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

    dir_create "${ANDROID_APP_BUILD_DIR}"
    runq "cd ${ANDROID_APP_BUILD_DIR}"
    
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
    local ARG9="-S ${BUILD_SOURCEDIRECTORY}"

    local ARGS="${ARG1} ${ARG2} ${ARG3} ${ARG4} ${ARG5} ${ARG6} ${ARG7} ${ARG8} ${ARG9}"
    runq "cmake $ARGS"

    runq make
    if [[ $? != 0 ]]; then
        warning "[ERROR] Make failed"
        exit 1
    fi

    run "cp ./src/c++/fuzzyvault/*.so ${ANDROID_LIB_DIR}/$outputFolder/lib"
    run "cp ./src/c++/fuzzyvault/*.a ${ANDROID_LIB_DIR}/$outputFolder/lib"
    run "cp ${BUILD_SOURCEDIRECTORY}/src/c++/fuzzyvault/fuzzy.h ${ANDROID_LIB_DIR}/$outputFolder/include/"
}

buildLibraries android-arm
buildLibraries android-x86_64
buildLibraries android-arm64
buildLibraries android-x86
