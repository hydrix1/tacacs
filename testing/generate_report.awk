BEGIN {
	n_errors = 0
	n_fails  = 0
        n_tests  = 0
        time_start = 0
        time_last = 0
        n_title = "test_suite"
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
        result = $2
        switch (result)
	{
            case "PASS":
		status = "Passed"
		break
            case "IGNORED":
		n_skips++
		status = "Skipped"
		break
	    default:
            case "FAIL":
		n_fails++
		status = "Failed"
		break
	}
        test_results[n_tests] = status
        test_classes[n_tests] = name
        test_names[n_tests] = test_name
        test_durations[n_tests] = time_last - time_mark
        test_output[n_tests] = output
	n_tests++
	next
       }
{
	line = gsub (/\\n/, "", $0)
	output = output line "\n"
}

END   {
        n_duration = time_last - time_start
	printf "<testsuite"
	printf " tests=\"%d\"", n_tests
	printf " skipped=\"%d\"", n_skips
	printf " errors=\"%d\"", n_errors
	printf " failures=\"%d\"", n_fails
	printf " name=\"%s\"", n_title
	printf " time=\"%5.3f\"", n_duration
	printf ">\n  <properties/>\n"
	printf "  <system-out>\n"
	printf "    -- There is no system-out\n"
	printf "  </system-out>\n"
	printf "  <system-err>\n"
	printf "    -- There is no system-err\n"
	printf "  </system-err>\n"
	for (test_idx = 0; test_idx < n_tests; test_idx++)
	{
	    printf "  <testcase"
	    printf   " classname=\"%s\"", test_classes[test_idx]
	    printf   " name=\"%s\"", test_names[test_idx]
	    printf   " status=\"%s\"", test_results[test_idx]
	    printf   " time=\"%5.3f\"", test_durations[test_idx]
	    printf   ">\n"
	    if (test_results[test_idx] == "Failed")
	    {
		printf "    <error"
		printf     " message=\"%s\"", test_err_msgs[test_id]
		printf     " type=\"%s\"", test_err_type[test_id]
		printf     ">\n"
		print test_output[test_id]
		printf "    </error>\n"
	    }
	    printf "  </testcase>\n"
	}
	printf "</testsuite>\n"
      }
