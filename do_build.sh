#!/bin/bash

# Find out where we are
script_name=$0
username=`whoami 2>/dev/null`
if [ $? -ne 0 ]; then
    username=`who am i | awk '{print $1}'`
fi
offset=`dirname $script_name`
called_from=`pwd`
base=$called_from/$offset
cd $base

echo "***"
echo "***"
echo "*** This is $username running $script_name"
echo "***   -- called from $called_from"
echo "***   -- working in  $base"
echo "***"
echo "***"

tac_start_script=~/tacacs_ssh_profile.sh
if [ -x $tac_start_script ]; then
    . $tac_start_script
fi

echo "***"
echo "***"
echo "*** No cheating!  delete executables if they exist"
echo "***"
echo "***"
sudo rm -fr /usr/local/sbin/tac_plus /tmp/tacacs

# Configure, build and install the original version
cd original

echo "***"
echo "***"
echo "*** Running 'configure' for original..."
echo "***"
echo "***"
./configure --prefix=/tmp/tacacs

echo "***"
echo "***"
echo "*** Running 'make' for original..."
echo "***"
echo "***"
make

echo "***"
echo "***"
echo "*** Running 'make install' for original..."
echo "***"
echo "***"
echo "*** If this fails, add $username to /etc/sudoers:"
echo "***          Defaults:$username   !requiretty"
echo "***          $username            ALL=(ALL) NOPASSWD:ALL"
echo "***"
sudo make install

# Configure, build and install the modified version
cd ../modified

echo "***"
echo "***"
echo "*** Running debug 'configure' for modified..."
echo "***"
echo "***"
export DEBUG=1
export RELEASE=0
./configure --prefix=/tmp/tacacs-debug --debug

echo "***"
echo "***"
echo "*** Running debug 'make' for modified..."
echo "***"
echo "***"
make clean
make

echo "***"
echo "***"
echo "*** Running debug 'make install' for modified..."
echo "***"
echo "***"
echo "*** If this fails, add $username to /etc/sudoers:"
echo "***          Defaults:$username   !requiretty"
echo "***          $username            ALL=(ALL) NOPASSWD:ALL"
echo "***"
sudo make install

echo "***"
echo "***"
echo "*** Running release 'configure' for modified..."
echo "***"
echo "***"
export DEBUG=0
export RELEASE=1
./configure

echo "***"
echo "***"
echo "*** Running release 'make' for modified..."
echo "***"
echo "***"
make clean
make

echo "***"
echo "***"
echo "*** Running release 'make install' for modified..."
echo "***"
echo "***"
echo "*** If this fails, add $username to /etc/sudoers:"
echo "***          Defaults:$username   !requiretty"
echo "***          $username            ALL=(ALL) NOPASSWD:ALL"
echo "***"
sudo make install

# Build the units tests
cd ../testing/unit

echo "***"
echo "***"
echo "*** Running 'make' for unit tests..."
echo "***"
echo "***"
gmake

echo "***"
echo "***"
echo "*** $username finished running $script_name"
echo "***"
echo "***"

