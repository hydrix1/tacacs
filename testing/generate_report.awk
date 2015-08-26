function reset_counters()
{
	n_errors = 0
	n_fails  = 0
        n_tests  = 0
}

function print_suite()
{
    if ((n_suite != "") && (n_tests > 0))
    {
        n_duration = time_last - time_start
	printf "  <testsuite"
	printf   " tests=\"%d\"", n_tests
	printf   " skipped=\"%d\"", n_skips
	printf   " errors=\"%d\"", n_errors
	printf   " failures=\"%d\"", n_fails
	printf   " package=\"%s\"", n_suite
	printf   " time=\"%5.3f\"", n_duration
	printf ">\n    <properties/>\n"
	for (test_idx = 0; test_idx < n_tests; test_idx++)
	{
	    printf "    <testcase"
	    printf     " classname=\"%s\"", test_classes[test_idx]
	    printf     " name=\"%s\"", test_names[test_idx]
	    printf     " status=\"%s\"", test_results[test_idx]
	    printf     " time=\"%5.3f\"", test_durations[test_idx]
	    printf     ">\n"
	    if (test_results[test_idx] == "Failed")
	    {
		printf "      <error"
		printf       " message=\"%s\"", test_err_msgs[test_idx]
		printf       " type=\"%s\"", test_err_type[test_idx]
		printf       ">\n"
		printf "      </error>\n"
	    }
	    printf "      <system-out>\n"
	    print test_output[test_idx]
	    printf "      </system-out>\n"
	    printf "    </testcase>\n"
	}
	printf "  </testsuite>\n"
    }
    reset_counters()
}

BEGIN {
	reset_counters()
        time_start = 0
        time_last = 0
        n_suite = ""
        n_title = "test_suite"
	printf "<testsuites>\n"
      }

{
	gsub (/\n/, "")
	gsub (/\r/, "")
}

/^@@@/ {
	time_last = $2
	if (time_start == 0)
	{
	    time_start = time_last
	}
	next
       }

/^%%%/ {
	time_mark = time_last
        test_name = $2
	output = ""
	next
       }

/^:::/ {
	name = substr($1, 4, length($1) - 6)
	n_parts = split(name, parts, "/")
	if (parts[1] != n_suite)
	{
	    print_suite()
	    n_suite = parts[1]
	}
        class = parts[2]
	for (c = 3; c < n_parts; c++)
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

{
	line = $0
	output = output "        " line "\n"
}

END   {
	print_suite()
	printf "</testsuites>\n"
      }
