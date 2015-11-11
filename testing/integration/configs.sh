#!/bin/bash
v#
# These integration tests are organised into a number of groups; each group
# is in its own directory underneath the "test_configs" directory in this
# directory.   For example, the "host" tests are all located in
# ./test_configs/host/.
#
# Each test comprises a set of files with the same basename, for example
# test "01_good" may comprise "01_good.script" amd "01_good.output".   These
# files contain various inputs to the test and expected outputs.   During
# testing, the "tac_plus" program is started using the specified command line
# arguments and stdin (/dev/null if none given) and the stdout/stderr are
# captured; after execution, the actual outputs are compared with the expected
# outputs to determine if the test has passed or not.
#
# The test files are:
#   .script:   the command line arguments used on invokation.  This file MUST
#              be present for each test.
#   .input:    text to be supplied as stdin, effectively providing the input
#              that an interactive user would provide during execution.   If
#              this file is not present for a particular test, /dev/null is
#              used instead.
#   .output:   if present, indicates that the test is expected to succeed
#              (i.e. exit with code 0) and produce the contents of this file
#              as standard output.
#   .error:    if this file is present, it indicates that the program is
#              expected to fail (exit with non-zero status) and produce
#              the contents of this file on stderr.
###################################################################################



###################################################################################
# 
# Run a single configuration integration test.
#
# Arguments:
#   $1: name of the test, for reporting purposes
#   $2: directory of where the test spec files are, relative to
#       testing/integration, for example "test_configs/host"
#   $3: base file name: all the test files start with this
#
tacacs_integration_test_one_config()
{
    name="$1"
    home="$2"
    base="$3"

    unity_start_test "${name}"
    base_result=`basename $home`

    stdin=${home}/${base}.input
    stdout=${home}/${base}.output
    stderr=${home}/${base}.error
    output=outputs/${base_result}_${name}.out
    errors=outputs/${base_result}_${name}.err
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



####################################################################################
#
# Run a group of configuration integration tests, which are all the
# tests in the specified ($1) directory
#
# Works by identifying each individual test ("XXX,script") and using the
# function above to run it.
#
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
#
# Run all the configuration integration tests
#
# Works by finding all the groups (test directories) and calling a function
# above to run each group in turn.
#
tacacs_config_integration_tests()
{
    mkdir -p outputs
    unity_start_group "config"
	for group in test_configs/*; do
	    tacacs_integration_test_configs $group
	done
    unity_end_group
}
