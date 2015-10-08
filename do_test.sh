#!/bin/bash

# Find out where we are
script_name=$0
username=`whoami`
offset=`dirname $script_name`
called_from=`pwd`
base=$called_from/$offset
cd $base

# Local definition
unit_test_output=unit_test_output.txt
intgn_test_output=intgn_test_output.txt
local_test_output=local_test_output.txt
remote_test_output=remote_test_output.txt
all_test_output=all_test_output.txt
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

# Run tests, capturing output
. scripts/run_all_tests.sh | tee $all_test_output

echo "***"
echo "***"
echo "*** Generate the report"
echo "***"
echo "***"
sed 's/\x1d/^]/g' $all_test_output > fixed_$all_test_output
gawk -f generate_report.awk fixed_$all_test_output > $xml_test_output

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

