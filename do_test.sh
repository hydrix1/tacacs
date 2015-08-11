#!/bin/bash

# define the test server and access
test_server=192.168.0.161
test_username=robot.build
test_password=Marvin

# Find out where we are
script_name=$0
username=`whoami`
offset=`dirname $script_name`
called_from=`pwd`
base=$called_from/$offset
cd $base

echo "***"
echo "***"
echo "*** This is $username running $script_name"
echo "***   -- called from $called_from"
echo "***   -- working in  $base"
echo "***"
echo "***"

# Connect to remote system to test this system
cd testing

echo "***"
echo "***"
echo "*** Starting test servers on local machine..."
echo "***"
echo "***"
task_list=""
for config in configs/*.server
do
    echo "+++ starting $config"
    $config &
    echo "+++ ---- task ID is $!"
    task_list="$! $task_list"
done


echo "***"
echo "***"
echo "*** Connecting to remote host $test_server as $test_username..."
echo "***"
echo "***"
sshpass -p $test_password ssh -t -t $test_username@$test_server <<EOF

echo "***"
echo "***"
echo "*** Running tests remotely..."
echo "***"
echo "***"
echo "***"
tacacs/testing/run_test.sh
exit
EOF

echo "***"
echo "***"
echo "*** Stopping test servers on local machine..."
echo "***"
echo "***"
for task in $task_list
do
    echo "+++ killing $task"
    kill -9 $task
done


echo "***"
echo "***"
echo "*** $username finished running $script_name"
echo "***"
echo "***"

