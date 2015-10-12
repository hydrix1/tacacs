#!/bin/bash
#
# Part of the  Tacacs+ test system.
#
# Configure a Cisco router to use the test TACACS+ server and
# test it by trying to access the router.
#

cisco_router_addr=192.168.1.24

cisco_lock_dir=/tmp/cisco.$cisco_router_addr.lockdir


##################################################################################################


# Grab exclusive use of Cisco router then run tests
tacacs_test_cisco()
{
    loops=0
    while [ $loops -lt 2000 ]; do
        loops=$(($loops + 1))
        if mkdir $cisco_lock_dir; then
            echo "$$" > $cisco_lock_dir/pid
            echo "$login@$machine" > $cisco_lock_dir/user
            python scripts/test_cisco_router.py -p $machine -r $cisco_router_addr
            rm -fr $cisco_lock_dir
            break;
        fi
        who=`cat $cisco_lock_dir/user`
        if [ $? != 0 ]; then
             who="unknown"
        fi
        echo "Cisco router busy by $who"
        sleep 20;
    done
}
