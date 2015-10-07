#!/bin/bash

NEW_TACPLUS=/usr/local/sbin/tac_plus
ORG_TACPLUS=/tmp/tacacs/sbin/tac_plus

echo "***"
echo "***"
echo "*** Start integration testing"
echo "***"
echo "***"

# Include tools from other scripts
. scripts/unity.sh

. integration/basic.sh


###########################################################################

# Run the unit tests themselves
tacacs_basic_integration_tests

