#!/bin/bash
#
# Script to run a series of basic integration tests.   Note that the test
# function is only defined here, not called.   This script is designed to
# be called from ../do_integration_test.sh only and will not work otherwise.
#
# The basic tests check two things:
#  - The expected binary files have been created by the make process
#  - The correct version numbers are printed.
#
###################################################################################


###################################################################################
#
# Check that the specified program ($2) exists and is executable.   The first
# parameter is an English descriptive name used for reporting.
#
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



###################################################################################
#
# Check that all the "tac_plus" programs are present and correct.   Specifically,
# check that both the original code and the modified code have both built.
#
tacacs_integration_test_progs_exist()
{
    unity_start_group "programs_exist"
	tacacs_integration_test_prog_exists original_tacplus $ORG_TACPLUS
	tacacs_integration_test_prog_exists new_tacplus $NEW_TACPLUS
    unity_end_group
}


###################################################################################



###################################################################################
#
# Confirm that running the original code with option "-v"
# prints the original (bare) version number.
#
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


###################################################################################
#
# Confirm that running the original code with option "--version"
# reports an error, as original program doesn't support long options
#
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



###################################################################################
#
# Confirm that running the modified code with option "-v"
# prints the new, extended version number, comprising the original
# version string and an indication that this is modified code,
#
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



###################################################################################
#
# Confirm that running the modified code with option "--version"
# prints the new, extended version number, comprising the original
# version string and an indication that this is modified code,
#
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



####################################################################################
#
# Check that the "tac_plus" program reports the correct version.   This breaks
# down into four specific tests, running the old and new programs with the
# the old "-v" and new "--version" options.   The expect results are:
#   original -v:         print original (bare) version number
#   original --version:  should error, as original program doesn't support
#                        long options
#   modified -v:         print extended version number, comprise the original
#                        version string and an indication that this is modified
#                        code,
#   modified --version:  same as "modified -v"
#
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



####################################################################################
#
# Run the all basic integration tests by invoking each in turn
#
tacacs_basic_integration_tests()
{
    unity_start_group "basic"
	tacacs_integration_test_progs_exist
	tacacs_integration_test_versions
    unity_end_group
}

