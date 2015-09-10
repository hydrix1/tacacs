#!/bin/bash
#
# Part of the  Tacacs+ test system.
#
# Run Tacacs+ tests from this machine via aJUNOS on a server running
# on a different machine.
#

router_addr=192.168.1.243

lock_dir=/tmp/junos.$router_addr.lockdir


##################################################################################################


# Grab exclusive use of JUNOS router then run tests
tacacs_test_junos()
{
    loops=0
    while [ $loops -lt 1000 ]; do
        loops=$(($loops + 1))
        if mkdir $lock_dir; then
            echo "$$" > $lock_dir/pid
            echo "$login@$machine" > $lock_dir/user
            python scripts/test_junos_router.py -p $machine -r $router_addr
            rm -fr $lock_dir
            break;
        fi
        who=`cat $lock_dir/user`
        if [ $? != 0 ]; then
             who="unknown"
        fi
        echo "Router busy by $who"
        sleep 10;
    done
}
