#!/bin/bash

echo "***"
echo "***"
echo "*** Start unit testing"
echo "***"
echo "***"

# Include tools from other scripts
. scripts/unity.sh

###########################################################################

#unity_start_group "Unit"

# Run the unit tests themselves
unity_start_test "dummy"
    if [ -x unit/dummy ]; then
	unit/dummy
	result=$?
	if [ $result != 123 ]; then
	    unity_fail
	    echo " -- exit code was $result; expected 123!"
	fi
    else
	unity_skip
	echo " -- unit test program 'dummy' not built!"
    fi
unity_end_test

# Run the unit tests themselves
unit_tests="string_append"
for unit_test in $unit_tests
do
    unity_start_test "$unit_test"
	if [ -x unit/$unit_test ]; then
	    unit/$unit_test
	    result=$?
	    if [ $result != 0 ]; then
		unity_fail
		echo " -- exit code was $result; expected 0!"
	    fi
	else
	    unity_skip
	    echo " -- unit test program '$unit_test' not built!"
	fi
    unity_end_test
done

#unity_end_group

