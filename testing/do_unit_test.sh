#!/bin/bash

echo "***"
echo "***"
echo "*** Start unit testing"
echo "***"
echo "***"

# Include tools from other scripts
. scripts/unity.sh

###########################################################################

# List the unit tests
unit_tests="string_append
            get_missing_argument
            get_omitted_argument
            get_required_argument
            get_optional_argument
            get_next_option
            parse_listen_subopts
            parse_cli_listen
            set_password
            parse_level_subopts
            parse_host_subopts
            parse_cli_debug
            parse_cli_host
            parse_cmd_subopts
            parse_group_subopts
            parse_cli_group
            get_user_shell
            parse_user_subopts
            parse_cli_user
            generate_more
            generate_listen_request
            generate_spawnd_config
            generate_level_config
            generate_level_set_config
            generate_host_config
            generate_access_config
            generate_cmd_config
            generate_service_config
            generate_group_config
            generate_user_config
            generate_tacplus_config
            generate_cli_config
           "

# Sanity check
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
for unit_test in $unit_tests
do
    if [ -x unit/$unit_test ]; then
	unit/$unit_test
	result=$?
        unity_start_test "$unit_test"
	if [ $result != 0 ]; then
	    unity_fail
	    echo " -- exit code was $result; expected 0!"
	fi
    else
        unity_start_test "$unit_test"
	unity_skip
	echo " -- unit test program '$unit_test' not built!"
    fi
    unity_end_test
done

