# generate_report.awk
#
# This script converts the text output to an XML form that Jenkins can parse.
#
# Note that neither the input nor the output formats are well defined, but
# individual components of the input are explained below.

# Simple subroutine to clear all the counters
function reset_counters()
{
    n_errors = 0
    n_fails  = 0
    n_tests  = 0
}

# Dump all the accumulated test results as a single "testcase"
function print_cases()
{
    for (test_idx = 0; test_idx < n_tests; test_idx++)
    {
        printf margin "  <testcase"
        printf            " classname=\"%s\"", test_classes[test_idx]
        printf            " name=\"%s\"", test_names[test_idx]
        printf            " status=\"%s\"", test_results[test_idx]
        printf            " time=\"%5.3f\"", test_durations[test_idx]
        printf            ">\n"
        if (test_results[test_idx] == "Failed")
        {
            printf margin "    <error"
            printf              " message=\"%s\"", test_err_msgs[test_idx]
            printf              " type=\"%s\"", test_err_type[test_idx]
            printf              "/>\n"
        }
        printf margin "    <system-out>\n"
        print test_output[test_idx]
        printf margin "    </system-out>\n"
        printf margin "  </testcase>\n"
    }
}

# Prepare to print out a suite of tests, if it is complete.
#
# "Complete" is determined by comparing the "path" or hierarchy of
# the test just completed with the path of the previous test; if the
# path has changed, we are moving from one suite to the next, so
# report all the accumulated tests and start gathering for the next
# suite.
#
# Note that we start and end with an empty path, which will be
# considered a change compared to any real test path.   Empty
# suites are not dumped.
#
function print_suite()
{
    if (1)
    {
        printf "<!-- " n_depth
        if (n_depth > 0)
        {
            printf " (" n_name[1]
            for (ii = 2; ii <= n_depth; ii++)
            {
                printf "." n_name[ii]
            }
            printf ")"
        }
        printf " -> " new_depth
        if (new_depth > 0)
        {
            printf " (" new_name[1]
            for (ii = 2; ii <= new_depth; ii++)
            {
                printf "." new_name[ii]
            }
            printf ")"
        }
        printf ", (" n_tests " tests) -->\n"
    }

    if ((n_depth > 0) || (new_depth > 0))
    {
        print_cases()

        while ((n_depth > new_depth) || ((n_depth > 0) && (n_name[n_depth] != new_name[n_depth])))
        {
            printf margin "  </testsuite>\n"
            n_depth--;
            margin = substr(margin, 3)
        }

        n_duration = time_last - time_start

        while (n_depth < new_depth)
        {
            n_depth++;
            n_name[n_depth] = new_name[n_depth]
            printf margin "  <testsuite"
            printf   " tests=\"%d\"", n_tests
            printf   " skipped=\"%d\"", n_skips
            printf   " errors=\"%d\"", n_errors
            printf   " failures=\"%d\"", n_fails
            printf   " package=\"%s", n_name[1]
            for (ii = 2; ii <= n_depth; ii++)
            {
                printf   ".%s", n_name[ii]
            }
            printf   "\""
            printf   " time=\"%5.3f\"", n_duration
            printf ">\n" margin "    <properties/>\n"
            margin = margin "  "
        }
    }
    reset_counters()
}

# Initialisation -- at the start:
#    clear all counters
#    initialise all variables
#    start the XML output
BEGIN {
    reset_counters()
    time_start = 0
    time_last = 0
    n_suite = ""
    n_depth = 0
    margin = ""
    n_title = "test_suite"
    printf "<testsuites>\n"
}

# General processing.   For all lines, get rid of CR/LF characters
# and <space><backspace> pairs from tunnel keep-alive messages
{
        gsub (/\n/, "")
        gsub (/\r/, "")
        gsub (/ \b/, "")
}

# Timing marks.
#
# From time to time, test programs/scripts will print a timing mark,
# which is denoted by a line starting with "@@@", followed by a time
# in seconds.   Here, we just remember the last time mark seen to
# allow other lines to access the latest time stamp.
/^@@@/ {
    time_last = $2
    if (time_start == 0)
    {
        time_start = time_last
    }
    next
}

# Start of test.
#
# When a test program or script is about to start a test, it dumps
# a timing mark (handled above) followed by a start of test marker,
# which is a line starting "%%%" followed by the pathless test name.
#
# We handle this by remembering the start time of the test and
# clearing the test output buffer.
/^%%%/ {
    time_mark = time_last
    test_name = $2
    output = ""
    next
}

# End of test.
#
# The end of a single test is maarked by a line starting ":::" followed
# by the full path name of the test, followed by a result word.
/^:::/ {
    name = substr($1, 4, length($1) - 6)
    n_parts = split(name, parts, "/")
    new_group = 0
    new_depth = n_parts - 1;
    for (c = 1; c <= new_depth; c++)
    {
        new_name[c] = parts[c]
        if (parts[c] != n_name[c])
        {
            new_group = 1
        }
    }
    if ((new_depth != n_depth) || new_group)
    {
        print_suite()
    }
    class = parts[1]
    for (c = 2; c < n_parts; c++)
    {
        class = class "." parts[c]
    }
    result = $2
    if (result == "PASS")
    {
        status = "Passed"
    }
    else if (result == "IGNORED")
    {
        n_skips++
        status = "Skipped"
    }
    else
    {
        n_fails++
        status = "Failed"
    }
    test_results[n_tests] = status
    test_classes[n_tests] = class
    test_names[n_tests] = test_name
    test_durations[n_tests] = time_last - time_mark
    test_output[n_tests] = output
    n_tests++
    next
}

# Non-special line.   If a line isn't special in any way, just add it to
# the current test's output log.   If we're not in a test, this will be
# thrown away later.
{
    line = $0
    output = output "        " line "\n"
}

# Termination -- clean up at end of input:
#  1. print out any remaining tests
#  2. Add an XML closer
END   {
    new_depth = 0
    print_suite()
    printf "</testsuites>\n"
}
