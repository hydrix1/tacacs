#!/bin/bash
#
# Part of the  Tacacs+ test system.
#
# Run Tacacs+ tests from this machine directly on a server running
# on a different machine.
#

declare -a win_pid_for_cyg_pid


tacacs_test_with_timeout()
{
    local timeout=$1
    local test_machine=$2
    local test_port=$3
    local test_user=$4
    local test_pass=$5
    local test_key=$6
    local test_extras="$7"
    local test_filter=$8

    "$tactest" $testhard -s $test_machine -port $test_port -u $test_user -p $test_pass -k $test_key $test_extras 2>&1 > $test_log&
    local test_id=$!
    local win_id=`ps -p $test_id | awk 'NR==2{print $4}'`
    ps -W
    echo "started $test_id($win_id): $tactest" $testhard -s $test_machine -port $test_port -u $test_user -p $test_pass -k $test_key $test_extras

    #wait for thread to finish
    echo "Waiting for $test_id($win_id)"
    local loop_time=0
    local wait_time=0
    while [ $loop_time -lt $timeout ]; do
        sleep 1
        wait_time=$(($wait_time + 1))
        if kill -0 $test_id 2>/dev/null; then
            # thread is still running
            loop_time=$wait_time
        else
            # thread has finished
            echo "@$wait_time: $test_id has exited"
            wait $test_id
            echo "@$wait_time: $test_id caught"
            test_id=0
            loop_time=$timeout
        fi
    done

    if [ $test_id -ne 0 ]; then
        echo "Given up waiting for $test_id($win_id)"
        taskkill /pid $win_id /t /f
        wait $test_id
    fi

    if [ "$test_filter" == "" ]; then
        cat $test_log
    else
        grep '\.\.\.' $test_log >$test_filter
        cat $test_filter
    fi
}



##################################################################################################

tacacs_test_authenicate_good()
{
    local username=$1
    local password=$2

    unity_start_test "good"
        tacacs_test_with_timeout 15 $machine $tacacs_port $username $password cisco -authen
        tacats_expect_true
    unity_end_test
}



tacacs_test_authenicate_username()
{
    local username=$1
    local password=$2

    unity_start_test "bad_username"
        tacacs_test_with_timeout 15 $machine $tacacs_port rubbish $password cisco -authen
        tacats_expect_false
    unity_end_test
}



tacacs_test_authenicate_password()
{
    local username=$1
    local password=$2

    unity_start_test "bad_password"
        tacacs_test_with_timeout 15 $machine $tacacs_port $username rubbish cisco -authen
        tacats_expect_false
    unity_end_test
}



tacacs_test_authenicate_key()
{
    local username=$1
    local password=$2

    unity_start_test "bad_key"
        tacacs_test_with_timeout 15 $machine $tacacs_port $username $password rubbish -authen
        tacats_expect_nothing
    unity_end_test
}



##################################################################################################


tacacs_test_acct_good()
{
    local username=$1
    local password=$2

    unity_start_test "good"
        tacacs_test_with_timeout 15 $machine $tacacs_port $username $password cisco "-acct start one=1"
        tacats_expect_true
    unity_end_test
}



##################################################################################################


tacacs_test_authorise_good()
{
    local username=$1
    local password=$2

    unity_start_test "good"
        tacacs_test_with_timeout 15 $machine $tacacs_port $username $password cisco -author
        tacats_expect_true
    unity_end_test
}



##################################################################################################


tacacs_test_authentication()
{
    local username=$1
    local password=$2

    unity_start_group "authentication"
        tacacs_test_authenicate_good $username $password
        tacacs_test_authenicate_username $username $password
        tacacs_test_authenicate_password $username $password
        tacacs_test_authenicate_key $username $password
    unity_end_group
}



tacacs_test_accounting()
{
    local username=$1
    local password=$2

    unity_start_group "accounting"
        tacacs_test_acct_good $username $password
    unity_end_group
}



tacacs_test_authorisation()
{
    local username=$1
    local password=$2

    unity_start_group "authorisation"
        tacacs_test_authorise_good $username $password
    unity_end_group
}



##################################################################################################

tacacs_test_simple_authenicate_good()
{
    unity_start_test "good"
        tacacs_test_with_timeout 15 $machine $tacacs_port test_${tacacs_port}_a A${tacacs_port} cisco -authen
        tacats_expect_true
    unity_end_test
}



tacacs_test_simple_authenicate_permit()
{
    unity_start_test "permit"
        tacacs_test_with_timeout 15 $machine $tacacs_port test_${tacacs_port}_b any_old_rubbish cisco -authen
        tacats_expect_true
    unity_end_test
}



tacacs_test_simple_authenicate_deny()
{
    unity_start_test "deny"
        tacacs_test_with_timeout 15 $machine $tacacs_port test_${tacacs_port}_c A${tacacs_port} cisco -authen
        tacats_expect_false
    unity_end_test
}



tacacs_test_simple_authentication()
{
    unity_start_group "authentication"
        tacacs_test_simple_authenicate_good
        tacacs_test_simple_authenicate_permit
        tacacs_test_simple_authenicate_deny
    unity_end_group
}


##################################################################################################


tacacs_test_simple()
{
    tacacs_port=$1
    local service=$2

    echo " -- Testing $service (port $tacacs_port)"
    unity_start_group "$service"
        tacacs_test_simple_authentication
        #tacacs_test_accounting
        #tacacs_test_authorisation
    unity_end_group
}



##################################################################################################


tacacs_test_server()
{
    tacacs_port=$1
    local user_pass=$2
    local service=$3

    echo " -- Testing $service (port $tacacs_port)"
    unity_start_group "$service"
        tacacs_test_authentication $user_pass $user_pass
        tacacs_test_accounting $user_pass $user_pass
        tacacs_test_authorisation $user_pass $user_pass
    unity_end_group
}



##################################################################################################


tacacs_test_perf_result()
{
    local file=$1
    local field="$2"
    awk "/^$field/"'{if ($NF == "secs") print int(1000*$(NF-1)); else print $NF}' $file
}

tacacs_test_perf_single()
{
    tacacs_port=$1
    local service=$2
    local filename=${tacacs_port}_tmp.$$
    local attempts=5000

    unity_start_test "${service}_single"
        time tacacs_test_with_timeout 100 $machine $tacacs_port test_${tacacs_port}_a A${tacacs_port} cisco "-c $attempts -authen" $filename
        local sent=$(tacacs_test_perf_result $filename "Total Commands")
        local good=$(tacacs_test_perf_result $filename "Successes")
        unity_assert_equal $attempts $sent
        unity_assert_equal $attempts $good
        cat $filename 
    unity_end_test
}



tacacs_test_perf_thread()
{
    local serial=$1
    local basename=$2
    local guard=$3
    local attempts=$4
    local filename=${serial}_${basename}
    local tempfile=${filename}.tmp

    #pre-test: runs a few cycles to ensure all threads are going
    echo $serial starting
    "$tactest" $testhard -s $machine -port $tacacs_port -u test_${tacacs_port}_a -p A${tacacs_port} -k cisco -c $guard -authen >/dev/null
    echo $serial warming up
    #The test proper
    "$tactest" $testhard -s $machine -port $tacacs_port -u test_${tacacs_port}_a -p A${tacacs_port} -k cisco -c $attempts -authen >$tempfile
    grep '\.\.\.' $tempfile >$filename
    rm -f $tempfile
    local sent=$(tacacs_test_perf_result $filename "Total Commands")
    local good=$(tacacs_test_perf_result $filename "Successes")
    unity_assert_equal $attempts $sent
    unity_assert_equal $attempts $good
    #post-test: runs a few cycles to ensure all threads are finished
    echo $serial cooling down
    "$tactest" $testhard -s $machine -port $tacacs_port -u test_${tacacs_port}_a -p A${tacacs_port} -k cisco -c $guard -authen >/dev/null
    echo $serial finished
}



tacacs_test_perf_parallel()
{
    tacacs_port=$1
    local service=$2
    local filename=${tacacs_port}_par.$$
    local guard=500
    local attempts=1500
    local threads=8
    local timeout=500

    unity_start_test "${service}_parallel"
        local id_list=""
        local wid_list=""
        #start all threads
        for ((idx=1; idx<=$threads; idx++)); do
            tacacs_test_perf_thread $idx $filename $guard $attempts &
            tid=$!
            wid=`ps -p $tid | awk 'NR==2{print $4}'`
            win_pid_for_cyg_pid[$tid]=$wid
            id_list="$id_list $tid"
            wid_list="$wid_list $tid($wid)"
        done
        ps -W
        #wait for all threads to finish
        sleep $threads
        echo "Waiting for $wid_list"
        local loop_time=0
        local wait_time=0
        while [ $loop_time -lt $timeout ]; do
            sleep 1
            old_list="$id_list"
            id_list=""
            loop_time=$timeout
            wait_time=$(($wait_time + 1))
            for tid in $old_list; do
                if kill -0 $tid 2>/dev/null; then
                    # thread is still running
                    id_list="$id_list $tid"
                    loop_time=$wait_time
                else
                    # thread has finished
                    echo "@$wait_time: $tid has exited"
                    wait $tid
                    echo "@$wait_time: $tid caught"
                    local max_timeout=$(($wait_time * 2 + 9))
                    if [ $timeout -gt $max_timeout ]; then
                        echo "reducing timeout from $timeout to $max_timeout"
                        timeout=$max_timeout
                    fi;
                fi
            done
        done
        for tid in $id_list; do
            wid=${win_pid_for_cyg_pid[$tid]}
            echo "Given up waiting for $tid($wid)"
            taskkill /pid $wid /t /f
            wait $tid
        done
        echo "All parallel tasks are now complete"
        #process results
        for ((idx=1; idx<=$threads; idx++)); do
            echo Check parallel thread $idx of $threads
            local this_file=${idx}_$filename
            cat $this_file
            local sent=$(tacacs_test_perf_result $this_file "Total Commands")
            local good=$(tacacs_test_perf_result $this_file "Successes")
            unity_assert_equal $attempts $sent
            unity_assert_equal $attempts $good
        done
    unity_end_test
}



tacacs_comp_serial_perf()
{
    local org_port=$1
    local new_port=$2
    local metric="$3"
    local margin=10

    local org_file=${org_port}_tmp.$$
    local new_file=${new_port}_tmp.$$

    unity_start_test "serial_${metric}"
        local lhs=$(tacacs_test_perf_result $org_file "$metric")
        local rhs=$(tacacs_test_perf_result $new_file "$metric")
        local diff_pc=$(( (($lhs - $rhs) * 200) / ($lhs + $rhs) ))
        if [ $diff_pc -lt 0 ]; then
            diff_pc=$((-$diff_pc))
        fi
        if [ $diff_pc -gt $margin ]; then
            echo "for $metric, $lhs and $rhs differ by $diff_pc%, more than $margin%"
            unity_fail
        else
            echo "for $metric, $lhs and $rhs differ by $diff_pc%, within $margin%"
        fi
    unity_end_test
}



tacacs_comp_parallel_perf()
{
    local org_port=$1
    local new_port=$2
    local metric="$3"
    local margin=10

    local org_base=${org_port}_par.$$
    local new_base=${new_port}_par.$$

    local org_file=1_${org_base}
    local new_file=1_${new_base}

    unity_start_test "parallel_${metric}"
        local lhs=$(tacacs_test_perf_result $org_file "$metric")
        local rhs=$(tacacs_test_perf_result $new_file "$metric")
        local diff_pc=$(( (($lhs - $rhs) * 200) / ($lhs + $rhs) ))
        if [ $diff_pc -lt 0 ]; then
            diff_pc=$((-$diff_pc))
        fi
        if [ $diff_pc -gt $margin ]; then
            echo "for $metric, $lhs and $rhs differ by $diff_pc%, more than $margin%"
            unity_fail
        else
            echo "for $metric, $lhs and $rhs differ by $diff_pc%, within $margin%"
        fi
    unity_end_test
}



tacacs_comp_perf()
{
    local org_port=$1
    local new_port=$2
    local service=$3

    unity_start_group "${service}"
        #metrics to compare?
        tacacs_comp_serial_perf $org_port $new_port "Time Taken"
        tacacs_comp_serial_perf $org_port $new_port "Avg Possible"
        tacacs_comp_parallel_perf $org_port $new_port "Time Taken"
        tacacs_comp_parallel_perf $org_port $new_port "Avg Possible"
    unity_end_group
}



tacacs_performance()
{
    local base_port=$1
    local service=$2

    echo " -- Measuring $service ($base_port series) performance"
    unity_start_group "${service}_series"
        tacacs_test_perf_single $(($base_port + 1)) "Original"
        tacacs_test_perf_single $(($base_port + 2)) "Modified"
        tacacs_test_perf_single $(($base_port + 3)) "CLI"
        tacacs_test_perf_parallel $(($base_port + 1)) "Original"
        tacacs_test_perf_parallel $(($base_port + 2)) "Modified"
        tacacs_test_perf_parallel $(($base_port + 3)) "CLI"
        tacacs_comp_perf $(($base_port + 1)) $(($base_port + 2)) "Original_vs_Modified"
        tacacs_comp_perf $(($base_port + 2)) $(($base_port + 3)) "Modified_vs_CLI"
    unity_end_group
}



##################################################################################################


tacacs_test_direct_simple()
{
    unity_start_group "Simple"
        tacacs_test_simple 4901 "Generic-Original"
        tacacs_test_simple 4902 "Generic-Modified"
        tacacs_test_simple 4903 "Generic-CLI"
        tacacs_test_simple 4911 "No-logging-Original"
        tacacs_test_simple 4912 "No-logging-Modified"
        tacacs_test_simple 4913 "No-logging-CLI"
        tacacs_test_simple 4921 "XR-Original"
        tacacs_test_simple 4922 "XR-Modified"
        tacacs_test_simple 4923 "XR-CLI"
    unity_end_group
}



tacacs_test_direct_server()
{
    unity_start_group "Server"
        tacacs_test_server 4901 cisco "Original"
        tacacs_test_server 4902 cisco "Modified"
        tacacs_test_server 4903 cisco "CLI"
        tacacs_test_server 4921 cisco "XR-Original"
        tacacs_test_server 4922 cisco "XR-Modified"
        tacacs_test_server 4923 cisco "XR-CLI"
        tacacs_test_server 4949 readonly "Reference"
        tacacs_test_server 4950 readonly "CLargs"
    unity_end_group
}



tacacs_test_direct_perf()
{
    unity_start_group "Performance"
        #tacacs_performance 4900 "general"
        tacacs_performance 4910 "No-logging"
        #tacacs_performance 4920 "XR"
    unity_end_group
}



tacacs_test_direct()
{
    testhard=""; #"-r 9 -w 2"

    tacacs_test_direct_simple
    tacacs_test_direct_server
    tacacs_test_direct_perf
}

