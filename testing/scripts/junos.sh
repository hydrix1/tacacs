#!/bin/bash
#
# Part of the  Tacacs+ test system.
#
# Run Tacacs+ tests from this machine via aJUNOS on a server running
# on a different machine.
#

router_addr=192.168.1.243
router_user=root
router_pass=hydrix1044a

lock_dir=/tmp/junos.$router_addr.lockdir


# Check we can talk to the router
tacacs_test_junos_comms()
{
    unity_start_test "comms"
        sshpass -p $router_pass ssh -t -t $router_user@$router_addr <<EOF | tee $test_log
exit
EOF
        echo "--- ***************************"
        ls -l $test_log
        echo "--- ***************************"
        cat -v $test_log
        echo "--- ***************************"
        tacacs_expect_text "^--- JUNOS "
        tacacs_dont_expect_text "Password incorrect"
        tacacs_expect_text "^logout"
    unity_end_test
}


##################################################################################################

tacacs_test_junos_configure_tacacs()
{
    use_tacacs_addr=$1
    use_tacacs_port=$2
    use_tacacs_key=$3

    sshpass -p $router_pass ssh -t -t $router_user@$router_addr <<EOF | tee $test_log
cli
edit
set system tacplus-server $use_tacacs_addr port $use_tacacs_port secret $use_tacacs_key
commit
exit
exit
exit
exit
EOF
    echo "--- ***************************"
    ls -l $test_log
    echo "--- ***************************"
    cat -v $test_log
    echo "--- ***************************"
    tacacs_expect_text "^--- JUNOS "
    tacacs_dont_expect_text "Password incorrect"
    tacacs_expect_text "^logout"
}


##################################################################################################

tacacs_test_junos_user()
{
    use_username=$1
    use_password=$2

    sshpass -p $use_password ssh -t -t $use_username@$router_addr <<EOF | tee $test_log
exit
EOF
    echo "--- ***************************"
    ls -l $test_log
    echo "--- ***************************"
    cat -v $test_log
    echo "--- ***************************"
    tacacs_expect_text "^--- JUNOS "
    tacacs_dont_expect_text "Password incorrect"
    tacacs_expect_text "^logout"
}


##################################################################################################

tacacs_test_junos_simple_access()
{
    tacacs_port=$1

    unity_start_test "access"
        tacacs_test_junos_configure_tacacs $machine $tacacs_port cisco
        #tacacs_test_junos_user "test_${tacacs_port}_a" "A${tacacs_port}"
        #tacacs_test_junos_user "test_${tacacs_port}_b" "B${tacacs_port}"
        #tacacs_test_junos_user "test_${tacacs_port}_c" "C${tacacs_port}"
        #tacacs_test_junos_user "test_${tacacs_port}_x" "X${tacacs_port}"
    unity_end_test
}


# Check the simple configuration
tacacs_test_junos_simple()
{
    tacacs_port=$1
    service=$2
    echo " -- Testing simple $service (port $tacacs_port)"
    unity_start_group "$service"
        tacacs_test_junos_simple_access $1
        #tacacs_test_accounting
        #tacacs_test_authorisation
    unity_end_group
}


##################################################################################################

# With exclusive use of JUNOS router, run tests
tacacs_test_junos_router()
{
    unity_start_group "JunOS"
        tacacs_test_junos_comms
        tacacs_test_junos_simple 4900 "reference"
    unity_end_group
}


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
