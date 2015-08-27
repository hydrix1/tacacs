#!/bin/bash
#
# Run Tacacs+ tests from this machine on a server running
# on a different machine.
#
# The results are text messages spoofing Unity
#
test_log=test.$$.out



mark_time()
{
    time_now=`date +%s.%N`
    echo "@@@ $time_now"
}


# Define Unity style helper functions
unity_start()
{
    declare -a test_groups
    test_depth=0
    test_context=""
    echo "### TACACS+ TEST SCRIPT STARTING ###"
    test_total=0
    test_fails=0
    test_skips=0
    test_max_length=0
}

unity_end()
{
    filler="----"
    while [ ${#filler} -lt $test_max_length ]; do
        filler="$filler-"
    done
    padded="Test"
    while [ ${#padded} -lt $test_max_length ]; do
        padded="$padded "
    done
    echo "### TEST SCRIPT FINISHED ###"
    echo "$test_total Tests $test_fails Failures $test_skips Ignored"
    echo "OK"
    echo ""
    echo "Test result summary"
    echo "   |--------|-$filler-|"
    echo "   | Result | $padded |"
    echo "   |--------|-$filler-|"
    for (( idx=0; idx<$test_total; idx++ )); do
        padded="${test_names[$idx]}"
        while [ ${#padded} -lt $test_max_length ]; do
            padded="$padded "
        done
        echo "   |  ${test_results[$idx]}  | $padded |"
    done
    echo "   |--------|-$filler-|"
    echo "End of summary"
    echo ""
}

unity_skip()
{
    skip_test=1
}

unity_fail()
{
    fail_test=1
}

unity_start_group()
{
    test_group[$test_depth]="$test_context"
    test_context="${test_context}$1/"
    test_depth=$(($test_depth + 1))
}

unity_end_group()
{
    if [ $test_depth > 0 ]; then
        test_depth=$(($test_depth - 1))
        test_context="${test_group[$test_depth]}"
    else
        echo "Error: too many unity_end_groups!"
    fi
}

unity_start_test()
{
    skip_test=0
    fail_test=0
    test_name=$1
    mark_time
    echo "%%% $test_name"
}

unity_end_test()
{
    test_title="$test_context$test_name"
    test_title_length=${#test_title}

    if [ $test_title_length -gt $test_max_length ]; then
        test_max_length=$test_title_length 
    fi

    if [ "$skip_test" == "1" ]; then
        test_result="IGNORED"
        test_skips=$(($test_skips + 1))
    elif [ "$fail_test" == "1" ]; then
        test_result="FAIL"
        test_fails=$(($test_fails + 1))
    else
        test_result="PASS"
    fi

    test_names[$test_total]="$test_title"
    test_results[$test_total]="$test_result"
    test_total=$(($test_total + 1))
    mark_time
    echo ":::$test_title::: $test_result"
}


unity_assert_equal()
{
    wanted="$1"
    got="$2"

    if [ "$wanted" != "$got" ]; then
        unity_fail
        echo " -- Wanted '$wanted', got '$got'!"
    fi
}



##################################################################################################


# Local, tacacs test specific, unity extensions
tacacs_expect_result()
{
    want_result=$1

    got_result=`grep "^Command Pass status " $test_log | sed 's/,/ /g' | awk '{print $5}'`
    unity_assert_equal "$want_result" "$got_result"
}



tacats_expect_true()
{
    tacacs_expect_result "True"
}



tacats_expect_false()
{
    tacacs_expect_result "False"
}



tacats_expect_nothing()
{
    tacacs_expect_result ""
}



##################################################################################################


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


# Find out provenance and other details
prog=$0
login=`whoami`
machine=`who am i | sed 's/[()]//g' | awk '{print $NF}'`

tactest="/cygdrive/c/Program Files (x86)/TACACS.net/tactest"
# Display our suspicions
echo "I am $login for $machine running $prog"

# Start "Unity" testing...
unity_start




# Test 1: the reference server
tacacs_test_simple 4900 "Simple"
tacacs_test_server 4949 "Reference"
tacacs_test_server 4950 "CLargs"




# All tests finished
unity_end

rm -f $test_log
