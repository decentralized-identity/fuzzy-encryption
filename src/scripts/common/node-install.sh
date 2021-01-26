#!/bin/bash

runq "which node"
if [ $? -ne 0 ]; then
    run "npm install -g n"
    run "n stable"
    runq "which node"
    if [ $? -ne 0 ]; then
        printf "${RED}    node failed to install${NC}\n"
        exit 1
    fi
else
    comment "    node detected"
fi
runq "which n"
if [ $? -ne 0 ]; then
    run "npm install -g n"
    run "n stable"
else
    comment "    n detected"
fi
NODE_VERSION=$(node --version)
if [[ "$NODE_VERSION" < "v14.15.1" ]]; then
    run "n 14.15.1"
    if [ $? -ne 0 ]; then
        printf "${RED}failed to upgrade node using n 14.15.1${NC}\n"
        exit 2
    fi
else
    comment "    node is up to date"
fi

