#!/bin/bash
#!/bin/sh

comment
comment "    build_ssh.sh"
comment

runq "mkdir -p ${ANDROID_BUILD_DIR}"
runq "cd ${ANDROID_BUILD_DIR}"
runq "rm -rf openssl"
run "git clone https://github.com/openssl/openssl.git"
run "cd ${ANDROID_OPENSSL_DIR}"

printf "${YELLOW}$ git checkout tags/openssl-3.0.0-${OPENSSL_MINOR_VERSION}${NC}\n"
git checkout tags/openssl-3.0.0-${OPENSSL_MINOR_VERSION}

if [ -z $API ]; then
    warning "No API version was provided"
    exit 1
fi

# Please refer to https://developer.android.com/ndk/guides/other_build_systems#autoconf for documentation

function buildLibraries {
    abi=$1
    comment
    comment "   build_ssl.buildLibraries $abi"
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
        warning "Unknown ABI: $abi"
        return
    fi

    debugArg=""
    if [[ $(echo "$BUILD" | tr '[:upper:]' '[:lower:]') == d*  ]]; then
        debugArg="-d"
    fi
    
    runq "./Configure $abi -D__ANDROID_API__="$API" $debugArg shared no-asm SYSROOT="$TOOLCHAIN/sysroot" -fstack-protector-strong"
    runq "make clean"
    runq "make update"
    runq "git clean -f"
    runq "make"

    runq "mkdir -p ${ANDROID_OPENSSL_DIR}/$outputFolder/lib"
    runq "cp ./*.so ${ANDROID_OPENSSL_DIR}/$outputFolder/lib"
    runq "cp ./*.a ${ANDROID_OPENSSL_DIR}/$outputFolder/lib"
    runq "cp -r ./include ${ANDROID_OPENSSL_DIR}/$outputFolder/"
}

runq "cd ${ANDROID_OPENSSL_DIR}"

buildLibraries android-arm
buildLibraries android-x86_64
buildLibraries android-arm64
buildLibraries android-x86
