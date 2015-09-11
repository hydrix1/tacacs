#!/bin/bash

# Find out where we are
script_name=$0
username=`whoami`
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
echo "*** Running 'configure' for modified..."
echo "***"
echo "***"
./configure

echo "***"
echo "***"
echo "*** Running 'make' for modified..."
echo "***"
echo "***"
make

echo "***"
echo "***"
echo "*** Running 'make install' for modified..."
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
make

echo "***"
echo "***"
echo "*** $username finished running $script_name"
echo "***"
echo "***"

