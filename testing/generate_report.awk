function reset_counters()
{
	n_errors = 0
	n_fails  = 0
        n_tests  = 0
}

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
	    printf   " package=\"%s\"", n_name[n_depth]
	    printf   " time=\"%5.3f\"", n_duration
	    printf ">\n" margin "    <properties/>\n"
	    margin = margin "  "
	}
    }
    reset_counters()
}

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

{
	line = $0
	output = output "        " line "\n"
}

END   {
	new_depth = 0
	print_suite()
	printf "</testsuites>\n"
      }
