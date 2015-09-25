#!/bin/bash
#
# Run Tacacs+ tests from this machine on a server running
# on a different machine.
#
# The results are text messages spoofing Unity
#
test_log=test.$$.out
init_dir=`pwd`
wd=`dirname $0`
cd $wd

# Include tools and tests from other scripts
. scripts/unity.sh
. scripts/expect.sh

. scripts/direct.sh
. scripts/cisco.sh
. scripts/junos.sh


##################################################################################################


# Find out provenance and other details
prog=$0
login=`whoami`
machine=`who am i | sed 's/[()]//g' | awk '{print $NF}'`
if [ "$1" != "" ]; then
    echo "(using supplied $1 instead of $machine)"
    machine=$1
fi

tactest="/cygdrive/c/Program Files (x86)/TACACS.net/tactest"
# Display our suspicions
echo "I am $login for $machine running $prog in $init_dir"

# Start "Unity" testing...
unity_start


# Test 1: the reference server
#tacacs_test_direct
tacacs_test_cisco
#tacacs_test_junos


# All tests finished
unity_end

rm -f $test_log

