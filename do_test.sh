#!/bin/bash

# Find out where we are
script_name=$0
username=`whoami`
offset=`dirname $script_name`
called_from=`pwd`
base=$called_from/$offset
cd $base

# Local definition
raw_test_output=raw_test_output.txt
xml_test_output=junit_test_output.xml

echo "***"
echo "***"
echo "*** This is $username running $script_name"
echo "***   -- called from $called_from"
echo "***   -- working in  $base"
echo "***"
echo "***"

# Connect to remote system to test this system
cd testing

# Include tools from other scripts
. scripts/unity.sh


echo "***"
echo "***"
echo "*** Empty any previous logs..."
echo "***"
echo "***"
rm -fr output
mkdir output

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

echo "***"
echo "***"
echo "*** Generate the report"
echo "***"
echo "***"
awk -f generate_report.awk $raw_test_output > $xml_test_output

# echo "***"
# echo "***"
# echo "*** Tidy up..."
# echo "***"
# echo "***"
# rm -fr output

echo "***"
echo "***"
echo "*** $username finished running $script_name"
echo "***"
echo "***"

