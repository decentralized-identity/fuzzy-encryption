#!/bin/bash
#!/bin/sh

set -e

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




# ARGUMENTS 
# Set this to your minSdkVersion.
export API=$1
export BUILD=$2

bash install_ndk.sh
bash build_ssl.sh $1 $2
bash build_android.sh $1 $2
