#!/usr/bin/bash

# Find out where we are
script_name=$0
username=`whoami
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

# Configure, build and install
cd PROJECTS

echo "***"
echo "***"
echo "*** Running 'configure'..."
echo "***"
echo "***"
./configure

echo "***"
echo "***"
echo "*** Running 'make'..."
echo "***"
echo "***"
make

echo "***"
echo "***"
echo "*** Running 'make install'..."
echo "***"
echo "***"
echo "*** If this fails, add $username to /etc/sudoers:"
echo "***          Defaults:$username   !requiretty"
echo "***          $username            ALL=(ALL) NOPASSWD:ALL"
echo "***"
sudo make install

echo "***"
echo "***"
echo "*** $username finished running $script_name"
echo "***"
echo "***"

