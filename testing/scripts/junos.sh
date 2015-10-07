#!/bin/bash
#
# Part of the  Tacacs+ test system.
#
# Configure a JunOS router to use the test TACACS+ servers
# sending them various SSH logins to check
#

junos_router_addr=192.168.1.243

junos_lock_dir=/tmp/junos.$junos_router_addr.lockdir


##################################################################################################


# Grab exclusive use of JUNOS router then run tests
tacacs_test_junos()
{
    loops=0
    while [ $loops -lt 1000 ]; do
        loops=$(($loops + 1))
        if mkdir $junos_lock_dir; then
            echo "$$" > $junos_lock_dir/pid
            echo "$login@$machine" > $junos_lock_dir/user
            python scripts/test_junos_router.py -p $machine -r $junos_router_addr
            rm -fr $junos_lock_dir
            break;
        fi
        who=`cat $junos_lock_dir/user`
        if [ $? != 0 ]; then
             who="unknown"
        fi
        echo "Router busy by $who"
        sleep 10;
    done
}
