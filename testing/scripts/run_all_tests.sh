#!/bin/bash

# Local definition
unit_test_output=unit_test_output.txt
intgn_test_output=intgn_test_output.txt
system_test_output=system_test_output.txt
remote_test_output=remote_test_output.txt

# Include tools from other scripts
. scripts/unity.sh


echo "***"
echo "***"
echo "*** Empty any previous logs..."
echo "***"
echo "***"
rm -fr ../output
mkdir ../output

# Start "unity testing,,,
unity_start

# Run unit tests:
unity_start_group "Unit"
. ./do_unit_test.sh
unity_end_group

# Run integration tests:
unity_start_group "Integration"
. ./do_integration_test.sh
unity_end_group

# Run system tests:
unity_start_group "System"
. ./do_system_test.sh
unity_end_group

# All tests finished
unity_end

# echo "***"
# echo "***"
# echo "*** Tidy up..."
# echo "***"
# echo "***"
# rm -fr ../output

echo "***"
echo "***"
echo "*** $username finished running $0"
echo "***"
echo "***"

