#!/bin/bash

set -e
runqs "apt-get update -y"
runqs "apt-get upgrade -y"
runqs "apt-get dist-upgrade -y"
runqs "apt-get install man -y"
runqs "apt-get install wget -y"
# run "apt install curl > /dev/null:
runqs "apt-get install perl -y"
runqs "apt-get install build-essential -y"
runqs "apt-get install git -y"
runqs "apt-get install python3 -y"
runqs "apt-get install cmake -y"
runqs "apt-get install python3-pip -y"
runqs "apt-get install default-jre -y"
install "wget"
install "unzip"
install "tar"
install "npm"
run "bash $COMMON_SCRIPTS_DIR/node-install.sh"
