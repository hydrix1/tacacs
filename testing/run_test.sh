#!/bin/bash
#
# Run Tacacs+ tests from this machine on a server running
# on a different machine.
#
# The results are text messages spoofing Unity
#
test_log=test.$$.out



# Define Unity style helper functions
unity_start()
{
    declare -a test_groups
    test_depth=0
    test_context=""
    echo "### UNITY STARTING ###"
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
    echo "### UNITY FINISHED ###"
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



tacacs_test_auth_good()
{
    unity_start_test "good"
        "$tactest" -s $machine -port $tacacs_port -u readonly -p readonly -k cisco -authen 2>&1 | tee $test_log
        tacats_expect_true
    unity_end_test
}



tacacs_test_auth_password()
{
    unity_start_test "bad_password"
        "$tactest" -s $machine -port $tacacs_port -u readonly -p readwrite -k cisco -authen 2>&1 | tee $test_log
        tacats_expect_false
    unity_end_test
}



tacacs_test_authentication()
{
    unity_start_group "authentication"
        tacacs_test_auth_good
        tacacs_test_auth_password
    unity_end_group
}



tacacs_test_service()
{
    echo " -- Testing $service (port $tacacs_port)"
    unity_start_group "$service"
        tacacs_test_authentication
    unity_end_group
}



tacacs_test_server()
{
    tacacs_port=$1
    service=$2
    tacacs_test_service
}



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
tacacs_test_server 4949 "Reference"
tacacs_test_server 4950 "CLargs"




# All tests finished
unity_end

rm -f $test_log

