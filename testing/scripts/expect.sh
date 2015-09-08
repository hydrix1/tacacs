#!/bin/bash
#
# Part of the  Tacacs+ test system.
#
# A mechanism akin to Unity to generate confirm results
#


# Local, tacacs test specific, unity extensions
tacacs_expect_text()
{
    want_text="$1"

    grep "$want_text" $test_log >/dev/null
    if [ "$?" != "0" ]; then
        unity_fail
        echo " -- didn't get $want_text"
    fi
}



tacacs_dont_expect_text()
{
    not_wanted_text="$1"

    grep "$not_wanted_text" $test_log >/dev/null
    if [ "$?" != "0" ]; then
        unity_fail
        echo " -- got unwanted $not_wanted_text"
    fi
}



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
