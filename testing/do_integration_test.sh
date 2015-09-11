#!/bin/bash

TACPLUS=/usr/local/sbin/tac_plus

echo "***"
echo "***"
echo "*** Start integration testing"
echo "***"
echo "***"

# Include tools from other scripts
. scripts/unity.sh

###########################################################################

#unity_start_group "Integration"

# Run the unit tests themselves
unity_start_test "null"
    if [ ! -f $TAC_PLUS ]; then
	unity_fail
	echo " -- TACACS+ program '$TACPLUS' doesn't exist!"
    elif [ ! -x $TAC_PLUS ]; then
	unity_fail
	echo " -- TACACS+ program '$TACPLUS' not executable!"
    fi
unity_end_test

#unity_end_group

