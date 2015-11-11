#!/bin/bash

# Find out where we are
script_name=$0
username=`whoami 2>/dev/null`
if [ $? -ne 0 ]; then
    username=`who am i | awk '{print $1}'`
fi
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

tac_start_script=~/tacacs_ssh_profile.sh
if [ -x $tac_start_script ]; then
    . $tac_start_script
fi

sudo /sbin/route add -net 172.16.1.0 netmask 255.255.255.0 gw 192.168.1.24

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

