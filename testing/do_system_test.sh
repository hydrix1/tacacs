#!/bin/bash

# define the test server and access
test_server=192.168.0.161
test_options=
test_username=robot.build

# Check if we are a cloud server using reverse tunnelling
this_machine=`hostname`
if [ "$this_machine" == "sovms102" ] || [ "$this_machine" == "sovms582" ]; then
    # Remote
    test_server=localhost
    test_options="-p 8022"
fi

# Include tools from other scripts
. scripts/unity.sh


expect_file()
{
    file_name="../$1"
    if [ ! -f "${file_name}" ]; then
	unity_fail
	echo " -- ERROR: file ${file_name} NOT created!"
    fi
} 


echo "***"
echo "***"
echo "*** Starting test servers on local machine..."
echo "***"
echo "***"
unity_start_group "start_servers"
log_list=""
server_list=""
task_list=""
for config in configs/*.server
do
    server=`basename $config .server`
    server_base="${server##*_}"
    unity_start_test "server_${server_base}"
	echo "+++ starting $config"
	$config &
        result=$?
        task_id=$!
	echo "+++ ---- task ID is $task_id"
	sleep 2
	kill -0 $task_id
	result=$?
	if [ $result != 0 ]; then
	    unity_fail
	    echo " -- server '$config' has died!"
	else
	    task_list="$task_id $task_list"
	    server_list="$server_list $server_base"
	    if grep "\.log" $config > /dev/null; then
		log_list="$log_list $server_base"
	    else
		echo "+++ ---- no logs expected for server $server_base"
	    fi
	fi
    unity_end_test
done
unity_end_group

echo "***"
echo "***"
echo "*** Connecting to remote host $test_options $test_server as $test_username..."
echo "***"
echo "***"
ssh -t -t $test_options $test_username@$test_server <<EOF | tee $remote_test_output

echo "***"
echo "***"
echo "*** Running tests remotely..."
echo "***"
echo "***"
echo "***"
tacacs/testing/run_test.sh $TEST_MACHINE_NAME
exit
EOF

echo "***"
echo "***"
echo "*** Stopping test servers on local machine..."
echo "***"
echo "***"
unity_start_group "stop_servers"
for task in $task_list
do
    unity_start_test "task_$task"
        echo "+++ killing $task"
        kill -9 $task
        result=$?
        if [ $result != 0 ]; then
	    unity_fail
	    echo " -- failed to stop task $task"
	fi
    unity_end_test
done
unity_end_group

echo "***"
echo "***"
echo "*** Check the logs..."
echo "***"
echo "***"
unity_start_group "check_logs"
for server in $log_list
do
    echo "+++ checking logs for server $server"
    unity_start_test "server_$server"
	expect_file "output/access_${server}.log"
	expect_file "output/acct_${server}.log"
    unity_end_test
done
unity_end_group
