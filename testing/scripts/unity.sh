#!/bin/bash
#
# Part of the  Tacacs+ test system.
#
# A mechanism akin to Unity to generate parsible test results
#


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
