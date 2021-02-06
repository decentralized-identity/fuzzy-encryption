#!/bin/bash
#!/bin/sh

comment
comment "    install_ndk.sh"
comment

which unzip > /dev/null
if [[ $? != 0 ]]; then
    runq "apt-get install unzip"
fi

ndkVersion="21.3.6528147"
filename=android-ndk-r21d-linux-x86_64.zip
sha=bcf4023eb8cb6976a4c7cff0a8a8f145f162bf4d
if [ ! -e $filename ]; then
    runq "wget https://dl.google.com/android/repository/$filename"
fi

comment "checking the hash of ${filename} ..."
actual=$(openssl sha1 -hex $filename | sed -E "s/SHA1\($filename\)= //" )
if [[ $sha != $actual ]]; then 
    warning "File downloaded has SHA1 '$actual', expected '$sha'"
    warning "If you believe this SHA1 is correct, please update this script."
    exit 1
else
    comment "File verified SHA1($actual)"
fi

if [ -d "${NDK_DIR}" ]; then
    runq "rm -rf ${NDK_DIR}"
fi
runq "mkdir -p ${NDK_DIR}"
runq "unzip $filename -d ${NDK_DIR}"

if [[ $? != 0 ]]; then
    warning "failed to unzip ${filename}"
    exit 2
fi
