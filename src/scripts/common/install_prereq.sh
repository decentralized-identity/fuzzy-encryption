#!/bin/bash

set -else
runq "apt-get update -y"
runq "apt-get upgrade -y"
runq "apt-get dist-upgrade -y"
runq "apt-get install man -y"
runq "apt-get install wget -y"
# run "apt install curl > /dev/null:
runq "apt-get install perl -y"
runq "apt-get install build-essential -y"
runq "apt-get install git -y"
runq "apt-get install python3 -y"
runq "apt-get install cmake -y"
runq "apt-get install python3-pip -y"
runq "apt-get install default-jre -y"
install "wget"
install "unzip"
install "tar"
install "npm"
run "bash $COMMON_SCRIPTS_DIR/node-install.sh"
