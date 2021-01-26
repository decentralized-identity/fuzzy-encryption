#!/bin/sh

which unzip > /dev/null
if [[ $? != 0 ]]; then
    echo "unzip not installed; installing..."
    apt-get install unzip
fi

ndkVersion="21.3.6528147"
filename=android-ndk-r21d-linux-x86_64.zip
sha=bcf4023eb8cb6976a4c7cff0a8a8f145f162bf4d
if [ ! -e $filename ]; then
    wget https://dl.google.com/android/repository/$filename
fi
actual=$(openssl sha1 -hex $filename | sed -E "s/SHA1\($filename\)= //" )
if [[ $sha != $actual ]]; then 
    echo "File downloaded has SHA1 '$actual', expected '$sha'"
    echo "If you believe this SHA1 is correct, please update this script."
    exit 1
else
    echo "File verified SHA1($actual)"
fi

mkdir -p /opt/android-sdk/ndk/
unzip -o $filename -d /opt/android-sdk/ndk/

if [[ $? != 0 ]]; then
    echo "failed to unzip $filename"
fi

git clone https://github.com/openssl/openssl.git
cd openssl
git checkout tags/openssl-3.0.0-alpha8
