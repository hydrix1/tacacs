#!/usr/bin/bash

# Find out where we are
iam=$0
offset=`dirname $iam`
called_from=`pwd`
base=$called_from/$offset
echo "I am [$iam]; here is [$offset]; wd is [$called_from]; base is [$base]"
cd $base

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
echo "*** If this fails, add `whoami` to /etc/sudoers; (I am `who am i`)"
echo "***          `whoami`   ALL = NOPASSWD: ALL"
sudo make install

