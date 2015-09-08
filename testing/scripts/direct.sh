#!/bin/bash
#
# Part of the  Tacacs+ test system.
#
# Run Tacacs+ tests from this machine directly on a server running
# on a different machine.
#


tacacs_test_authenicate_good()
{
    unity_start_test "good"
        "$tactest" -s $machine -port $tacacs_port -u readonly -p readonly -k cisco -authen 2>&1 | tee $test_log
        tacats_expect_true
    unity_end_test
}



tacacs_test_authenicate_username()
{
    unity_start_test "bad_username"
        "$tactest" -s $machine -port $tacacs_port -u rubbish -p readonly -k cisco -authen 2>&1 | tee $test_log
        tacats_expect_false
    unity_end_test
}



tacacs_test_authenicate_password()
{
    unity_start_test "bad_password"
        "$tactest" -s $machine -port $tacacs_port -u readonly -p rubbish -k cisco -authen 2>&1 | tee $test_log
        tacats_expect_false
    unity_end_test
}



tacacs_test_authenicate_key()
{
    unity_start_test "bad_key"
        "$tactest" -s $machine -port $tacacs_port -u readonly -p readonly -k rubbish -authen 2>&1 | tee $test_log
        tacats_expect_nothing
    unity_end_test
}



##################################################################################################


tacacs_test_acct_good()
{
    unity_start_test "good"
        "$tactest" -s $machine -port $tacacs_port -u readonly -k cisco -acct start one=1 2>&1 | tee $test_log
        tacats_expect_true
    unity_end_test
}



##################################################################################################


tacacs_test_authorise_good()
{
    unity_start_test "good"
        "$tactest" -s $machine -port $tacacs_port -u readonly -k cisco -author 2>&1 | tee $test_log
        tacats_expect_true
    unity_end_test
}



##################################################################################################


tacacs_test_authentication()
{
    unity_start_group "authentication"
        tacacs_test_authenicate_good
        tacacs_test_authenicate_username
        tacacs_test_authenicate_password
        tacacs_test_authenicate_key
    unity_end_group
}



tacacs_test_accounting()
{
    unity_start_group "accounting"
        tacacs_test_acct_good
    unity_end_group
}



tacacs_test_authorisation()
{
    unity_start_group "authorisation"
        tacacs_test_authorise_good
    unity_end_group
}



##################################################################################################

tacacs_test_simple_authenicate_good()
{
    unity_start_test "good"
        "$tactest" -s $machine -port $tacacs_port -u test_4900_a -p A4900 -k cisco -authen 2>&1 | tee $test_log
        tacats_expect_true
    unity_end_test
}



tacacs_test_simple_authenicate_permit()
{
    unity_start_test "permit"
        "$tactest" -s $machine -port $tacacs_port -u test_4900_b -p any_old_rubbish -k cisco -authen 2>&1 | tee $test_log
        tacats_expect_true
    unity_end_test
}



tacacs_test_simple_authenicate_deny()
{
    unity_start_test "deny"
        "$tactest" -s $machine -port $tacacs_port -u test_4900_c -p A4900 -k cisco -authen 2>&1 | tee $test_log
        tacats_expect_false
    unity_end_test
}



tacacs_test_simple_authentication()
{
    unity_start_group "authentication"
        tacacs_test_simple_authenicate_good
        tacacs_test_simple_authenicate_permit
        tacacs_test_simple_authenicate_deny
    unity_end_group
}


##################################################################################################


tacacs_test_simple()
{
    tacacs_port=$1
    service=$2
    echo " -- Testing $service (port $tacacs_port)"
    unity_start_group "$service"
        tacacs_test_simple_authentication
        #tacacs_test_accounting
        #tacacs_test_authorisation
    unity_end_group
}



##################################################################################################


tacacs_test_server()
{
    tacacs_port=$1
    service=$2
    echo " -- Testing $service (port $tacacs_port)"
    unity_start_group "$service"
        tacacs_test_authentication
        tacacs_test_accounting
        tacacs_test_authorisation
    unity_end_group
}



##################################################################################################


tacacs_test_direct()
{
    tacacs_test_simple 4900 "Simple"
    tacacs_test_server 4949 "Reference"
    tacacs_test_server 4950 "CLargs"
}

