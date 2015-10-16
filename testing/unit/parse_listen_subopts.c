/*
 * Unit test program for parse_listen_subopts()
 */

#include <stdlib.h>
#include "unity.h"
#include "wrap_spawnd_main.h"


struct test_case
{
    const char*  name;
    int          arg_count;
    const char*  arg_values[10];
};



static void dump_listen_config (const struct listen_config* lc)
{
    for (; lc != 0; lc = lc->next_listen)
    {
        printf ("Listen config:\n");
        printf ("    next_listen %s null\n", lc->next_listen ? "!=" : "==");
        printf ("    port = ``%s''\n", lc->port);
        printf ("----\n");
    }
}


static int trace_system(const char* cmd)
{
    int result;
    fprintf (stderr, "-- running ``%s''\n", cmd);
    result = system (cmd);
    fprintf (stderr, "    -- returned %d\n", result);
    return result;
}



static void catch_test_parse_listen_subopts(const struct test_case* this_test)
{
    pid_t parent;
    pid_t child;
    char* file_base = 0;
    char* in_file = 0;
    char* expected_result = 0;
    char* expected_output = 0;
    char* expected_error = 0;
    char* actual_result = 0;
    char* actual_output = 0;
    char* actual_error = 0;
    char* make_actual_dir = 0;
    char* cmp_results = 0;
    char* cmp_outputs = 0;
    char* cmp_errors = 0;

    unity_start_test(this_test->name);
    parent = getpid();

    asprintf(&file_base, "parse_listen_subopts_%s", this_test->name);
    asprintf(&in_file, "unit/inputs/%s.input", file_base);
    asprintf(&expected_result, "unit/expect/%s.result", file_base);
    asprintf(&expected_output, "unit/expect/%s.output", file_base);
    asprintf(&expected_error, "unit/expect/%s.error", file_base);
    asprintf(&actual_result, "unit/got_%d/%s.result", parent, file_base);
    asprintf(&actual_output, "unit/got_%d/%s.output", parent, file_base);
    asprintf(&actual_error, "unit/got_%d/%s.error", parent, file_base);
    asprintf(&make_actual_dir, "mkdir -p unit/got_%d", parent);
    asprintf(&cmp_results, "diff -bw %s %s", expected_result, actual_result);
    asprintf(&cmp_outputs, "diff -bw %s %s", expected_output, actual_output);
    asprintf(&cmp_errors, "diff -bw %s %s", expected_error, actual_error);

    trace_system(make_actual_dir);

    child = fork();
    if (child == -1)
    {
        /* fork() failed! */
        fprintf(stderr, "fork() failed!\n");
        unity_fail();
    }
    else if (child == 0)
    {
        /* we are the child process */
        struct listen_config  result;
        int                   next_sym;

        unlink (actual_result);
        freopen (in_file, "r", stdin);
        freopen (actual_output, "w", stdout);
        freopen (actual_error, "w", stderr);

        opts_argc = this_test->arg_count;
        opts_argv = (char**) this_test->arg_values;
        optind = 0;
        memset (&result, 0, sizeof(result));
        next_sym = parse_listen_subopts(&result);

        /* We may nor may not get here! */
        freopen (actual_result, "w", stdout);
        printf ("Next symbol = %d\n", next_sym);
        dump_listen_config (&result);
        exit(0);
    }
    else
    {
        /* we are the parent */
        int status = 0;
        int delta;
        int expect_to_work = 0;
        struct stat file_stat;

        /* Do we expect this oprtation to work? */
        delta = stat(expected_result, &file_stat);
        if (delta < 0)
        {
            /* No expected result file ==> expect failure */
             expect_to_work = 0;
        }
        else
        {
            /* There ia an expected result file ==> expect success */
             expect_to_work = 1;
        }

        /* wait for the child to exit */
        pid_t from = waitpid (child, &status, 0);
        /* confirm status is from our child */
        //fprintf(stderr, "waitpid(%d) response from %d\n", child, from);
        unity_assert_int_equal(child, from);
        //fprintf(stderr, "wait() exited with code %d\n", status);
        /* Check exit status is "exited" ...*/
        unity_assert_int_equal(1, WIFEXITED(status));
        /* ... and that the return value is as expected */
        if (expect_to_work)
        {
            unity_assert_int_equal(0, WEXITSTATUS(status));
            delta = trace_system(cmp_results);
            unity_assert_int_equal(0, delta);
        }
        else
        {
            unity_assert_int_not_equal(0, WEXITSTATUS(status));
        }

        /* Confirm the output and error files are as expected */
        delta = trace_system(cmp_outputs);
        unity_assert_int_equal(0, delta);
        delta = trace_system(cmp_errors);
        unity_assert_int_equal(0, delta);
    }

    free(file_base);
    free(in_file);
    free(expected_result);
    free(expected_output);
    free(expected_error);
    free(actual_result);
    free(actual_output);
    free(actual_error);
    free(make_actual_dir);
    free(cmp_results);
    free(cmp_outputs);
    free(cmp_errors);

    unity_end_test();
}



static const struct test_case case_table[] =
{
    { "null",         1, { "prog", 0 } },
    { "nothing",      2, { "prog", "--user", 0 } },
    { "correct",      3, { "prog", "--listen_port=9876", "--user", 0 } },
    { "no_value",     3, { "prog", "--listen_port", "--user", 0 } },
    { "duplicate",    4, { "prog", "--listen_port=9876", "--listen_port=1234", "--user", 0 } },
    { 0 }
};



void test_entry()
{
    int test_index;

    /* Test parse_listen_subopts() */
    unity_start_group("parse_listen_subopts");
    for (test_index = 0; case_table[test_index].name != 0; ++test_index)
    {
        catch_test_parse_listen_subopts(&case_table[test_index]);
    }
    unity_end_group();
}

