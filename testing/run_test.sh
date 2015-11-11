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
login=`whoami 2>/dev/null`
if [ $? -ne 0 ]; then
    login=`who am i | awk '{print $1}'`
fi
machine=`who am i | sed 's/[()]//g' | awk '{print $NF}'`
if [ "$1" != "" ]; then
    echo "(using supplied $1 instead of $machine)"
    machine=$1
fi

tactest="/cygdrive/c/Program Files (x86)/TACACS.net/tactest"
# Display our suspicions
echo "I am $login for $machine running $prog in $init_dir"

echo "ensuring reverse ssh tunnels are up"
echo "(errors may be reported if the tunnels are already up -- please ignore)"
ssh -C -g -f -T -R 8022:localhost:22 -p 58222 hydrix@cloud.siteox.com ./keep_open.sh
ssh -C -g -f -T -R 8022:localhost:22 -p 10222 hydrix@cloud.siteox.com ./keep_open.sh

# Start "Unity" testing...
unity_start


# Test 1: the reference server
tacacs_test_direct
#tacacs_test_cisco
#tacacs_test_junos


# All tests finished
unity_end

rm -f $test_log

