#!/bin/bash

# define the test server and access
test_server=192.168.0.161
test_username=robot.build



expect_file()
{
    file_name="$1"
    if [ ! -f "${file_name}" ]; then
       echo "*** ERROR: file ${file_name} NOT created!"
    fi
} 


echo "***"
echo "***"
echo "*** Starting test servers on local machine..."
echo "***"
echo "***"
server_list=""
task_list=""
for config in configs/*.server
do
    echo "+++ starting $config"
    $config &
    echo "+++ ---- task ID is $!"
    server=`basename $config .server`
    server_list="$server_list ${server##*_}"
    task_list="$! $task_list"
done


echo "***"
echo "***"
echo "*** Connecting to remote host $test_server as $test_username..."
echo "***"
echo "***"
ssh -t -t $test_username@$test_server <<EOF | tee $raw_test_output

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
echo "*** Check the logs..."
echo "***"
echo "***"
for server in $server_list
do
    echo "+++ checking logs for server $server"
    expect_file "output/access_${server}.log"
    expect_file "output/acct_${server}.log"
done

