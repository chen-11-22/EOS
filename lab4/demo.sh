#!/bin/sh

set -x #prints each command and its arguments to the terminal before executing it
# set -e #Exit immediately if a command exits with a non-zero status

rmmod -f /dev/my_dev
insmod name_driver.ko

./writer chen & #run in subshell
./reader 192.168.222.4 22 /dev/my_dev


