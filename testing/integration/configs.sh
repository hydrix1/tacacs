#!/bin/bash

###################################################################################


tacacs_integration_test_prog_exists()
{
    expected=$1
    unity_start_test "${expected}_exist"
	if [ ! -f $expected ]; then
	    unity_fail
	    echo " -- TACACS+ program '$expected' doesn't exist!"
	elif [ ! -x $expected ]; then
	    unity_fail
	    echo " -- TACACS+ program '$expected' not executable!"
	fi
    unity_end_test
}

tacacs_integration_test_one_config()
{
    name="$1"
    home="$2"
    base="$3"

    unity_start_test "${name}"

    stdin=${home}/${base}.input
    stdout=${home}/${base}.output
    stderr=${home}/${base}.error
    output=outputs/${home////_}_${name}.$$.out
    errors=outputs/${home////_}_${name}.$$.err
    script=${home}/${base}.script

    args="--check --print $(<$script)"

    if [ ! -f $stdin ]; then
        stdin=/dev/null
    fi

    echo -- running: /usr/local/sbin/tac_plus $args
    eval /usr/local/sbin/tac_plus $args > $output 2> $errors < $stdin
    result=$?

    if [ -f $stdout ]; then
        diff -b -w $stdout ${output} > ${output}.diffs
        delta=$?
        if [ $delta -ne 0 ]; then
            unity_fail
            echo " unexpected stdout!"
        fi
    fi

    if [ -f $stderr ]; then
        if [ $result -eq 0 ]; then
            unity_fail
            echo " unexpected zero exit code!"
        fi
        diff -b -w $stderr ${errors} > ${errors}.diffs
        delta=$?
        if [ $delta -ne 0 ]; then
            unity_fail
            echo " unexpected stderr!"
        fi
    else
        if [ $result -ne 0 ]; then
            unity_fail
            echo " non-zero exit code ($result)!"
        fi
    fi

    unity_end_test
}

tacacs_integration_test_configs()
{
    dir="$1"
    group=`basename $1`
    unity_start_group "$group"
        for script in ${dir}/*.script; do
	    base=`basename $script .script`
	    name=${base##$root}
	    tacacs_integration_test_one_config "$name" "$dir" "$base"
	done
    unity_end_group
}


####################################################################################

tacacs_config_integration_tests()
{
    mkdir -p outputs
    unity_start_group "config"
	for group in test_configs/*; do
	    tacacs_integration_test_configs $group
	done
    unity_end_group
}
