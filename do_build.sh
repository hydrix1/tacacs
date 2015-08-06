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
./configure
make
sudo make install

