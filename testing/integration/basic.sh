#!/bin/bash

###################################################################################


tacacs_integration_test_prog_exists()
{
    name=$1
    expected=$2
    unity_start_test "${name}_exists"
	if [ ! -f $expected ]; then
	    unity_fail
	    echo " -- TACACS+ program '$expected' doesn't exist!"
	elif [ ! -x $expected ]; then
	    unity_fail
	    echo " -- TACACS+ program '$expected' not executable!"
	fi
    unity_end_test
}

tacacs_integration_test_progs_exist()
{
    unity_start_group "programs_exist"
	tacacs_integration_test_prog_exists original_tacplus $ORG_TACPLUS
	tacacs_integration_test_prog_exists new_tacplus $NEW_TACPLUS
    unity_end_group
}


###################################################################################


tacacs_integration_test_old_short_version()
{
    program=$1
    expected_version=$2
    unity_start_test "original_short_version"
	answer=`$program -v 2>&1`
	result=$?
        x=`grep Usage: <<<$answer >/dev/null`
        got_usage=$?
        x=`grep $expected_version <<<$answer >/dev/null`
        got_version=$?
        x=`grep -i $expected_version/CLI <<<$answer >/dev/null`
        got_extra=$?
	if [ $result != 0 ]; then
	    unity_fail
	    echo " -- $program -v failed!"
	fi
	if [ $got_usage == 0 ]; then
	    unity_fail
	    echo " -- $program -v reported usage!"
	fi
	if [ $got_version != 0 ]; then
	    unity_fail
	    echo " -- $program -v had unexpected version!"
	fi
	if [ $got_extra == 0 ]; then
	    unity_fail
	    echo " -- $program -v reported CLI!"
	fi
    unity_end_test
}

tacacs_integration_test_old_long_version()
{
    program=$1
    expected_version=$2
    unity_start_test "original_long_version"
	answer=`$program --version 2>&1`
	result=$?
        x=`grep Usage: <<<$answer >/dev/null`
        got_usage=$?
        x=`grep -i $expected_version/CLI <<<$answer >/dev/null`
        got_extra=$?
	if [ $result == 0 ]; then
	    unity_fail
	    echo " -- $program --version didn't fail!"
	fi
	if [ $got_usage != 0 ]; then
	    unity_fail
	    echo " -- $program --version didn't report usage!"
	fi
	if [ $got_extra == 0 ]; then
	    unity_fail
	    echo " -- $program --version reported CLI!"
	fi
    unity_end_test
}

tacacs_integration_test_new_short_version()
{
    program=$1
    expected_version=$2
    unity_start_test "modified_short_version"
	answer=`$program -v 2>&1`
	result=$?
        x=`grep Usage: <<<$answer >/dev/null`
        got_usage=$?
        x=`grep $expected_version <<<$answer >/dev/null`
        got_version=$?
        x=`grep -i $expected_version/CLI <<<$answer >/dev/null`
        got_extra=$?
	if [ $result != 0 ]; then
	    unity_fail
	    echo " -- $program -v failed!"
	fi
	if [ $got_usage == 0 ]; then
	    unity_fail
	    echo " -- $program -v reported usage!"
	fi
	if [ $got_version != 0 ]; then
	    unity_fail
	    echo " -- $program -v had unexpected version!"
	fi
	if [ $got_extra != 0 ]; then
	    unity_fail
	    echo " -- $program -v didn't report CLI!"
	fi
    unity_end_test
}

tacacs_integration_test_new_long_version()
{
    program=$1
    expected_version=$2
    unity_start_test "modified_long_version"
	answer=`$program --version 2>&1`
	result=$?
        x=`grep Usage: <<<$answer >/dev/null`
        got_usage=$?
        x=`grep $expected_version <<<$answer >/dev/null`
        got_version=$?
        x=`grep -i $expected_version/CLI <<<$answer >/dev/null`
        got_extra=$?
	if [ $result != 0 ]; then
	    unity_fail
	    echo " -- $program --version failed!"
	fi
	if [ $got_usage == 0 ]; then
	    unity_fail
	    echo " -- $program --version reported usage!"
	fi
	if [ $got_version != 0 ]; then
	    unity_fail
	    echo " -- $program --version had unexpected version!"
	fi
	if [ $got_extra != 0 ]; then
	    unity_fail
	    echo " -- $program --version didn't report CLI!"
	fi
    unity_end_test
}

tacacs_integration_test_versions()
{
    expected="201503290942"
    unity_start_group "versions"
	tacacs_integration_test_old_short_version $ORG_TACPLUS $expected
	tacacs_integration_test_old_long_version $ORG_TACPLUS $expected
	tacacs_integration_test_new_short_version $NEW_TACPLUS $expected
	tacacs_integration_test_new_long_version $NEW_TACPLUS $expected
    unity_end_group
}


####################################################################################

tacacs_basic_integration_tests()
{
    unity_start_group "basic"
	tacacs_integration_test_progs_exist
	tacacs_integration_test_versions
    unity_end_group
}

