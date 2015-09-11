#!/bin/bash

###################################################################################


tacacs_integration_test_progs_exist()
{
    unity_start_test "programs_exist"
	if [ ! -f $TAC_PLUS ]; then
	    unity_fail
	    echo " -- TACACS+ program '$TACPLUS' doesn't exist!"
	elif [ ! -x $TAC_PLUS ]; then
	    unity_fail
	    echo " -- TACACS+ program '$TACPLUS' not executable!"
	fi
    unity_end_test
}


####################################################################################

tacacs_basic_integration_tests()
{
    unity_start_group "basic"
	tacacs_integration_test_progs_exist
    unity_end_group
}

